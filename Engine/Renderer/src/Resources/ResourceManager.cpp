#include "ResourceManager.h"
#include "Exception.h"
#include "GPUBuffer.h"
#include "../Command/CommandQueueManager.h"
#include "../Command/CommandContext.h"

#include <cassert>


namespace renderer::core
{
	renderer::core::ResourceHandle ResourceManager::GetStaticResourceHandleInternal()
	{
		if (!freeIndices_.empty())
		{
			uint32_t index = freeIndices_.front();
			freeIndices_.pop();

			return renderer::core::ResourceHandle{
				(static_cast<uint64_t>(staticResourceSlots_[index].version) << 32) | static_cast<uint64_t>(index) };
		}

		uint32_t index = staticResourceSlots_.size();
		staticResourceSlots_.emplace_back(nullptr, 1, false);
		return renderer::core::ResourceHandle{
			(static_cast<uint64_t>(staticResourceSlots_[index].version) << 32) | static_cast<uint64_t>(index) };
	}
}
// Constructor
namespace renderer::core
{
	ResourceManager::ResourceManager(ID3D12Device8* device, renderer::command::CommandQueueManager* manager) :device_(device), manager_(manager)
	{
		assert(device_ != nullptr && "ResourceManager needs a available ID3D12Device8 pointer.");
		assert(manager_ != nullptr && "ResourceManager needs a available CommandQueueManager pointer.");
	}
	ResourceManager::~ResourceManager() = default;
}
namespace renderer::core
{
	template <typename TCommandContext>
	renderer::core::ResourceHandle ResourceManager::CreateStaticBuffer(
		TCommandContext& context, const void* data, size_t dataByteSize)
	{
		std::lock_guard lock(staticResourceMutex_);

		renderer::core::ResourceHandle returnHandle = GetStaticResourceHandleInternal();

		std::unique_ptr<renderer::resources::IGPUResource> destBuffer =
			std::make_unique<renderer::resources::GPUBuffer>(
				device_, dataByteSize, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_COPY_DEST);

		if (data != nullptr)
		{
			std::unique_ptr<renderer::resources::GPUBuffer> uploadBuffer = std::make_unique<renderer::resources::GPUBuffer>(
				device_, dataByteSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_COMMON);

			uploadBuffer.get()->CopyData(data, dataByteSize, 0);
			context.CopyBuffer(destBuffer->GetResource(), uploadBuffer->GetResource(), dataByteSize);

			manager_->SafeRelease(std::move(uploadBuffer));
		}

		staticResourceSlots_[returnHandle.GetIndex()].resource = std::move(destBuffer);
		staticResourceSlots_[returnHandle.GetIndex()].version = returnHandle.GetVersion();
		staticResourceSlots_[returnHandle.GetIndex()].referenceCount = 1;
		staticResourceSlots_[returnHandle.GetIndex()].isFree = false;

		return returnHandle;
	}
	void ResourceManager::DestroyStaticBuffer(renderer::core::ResourceHandle handle)
	{
		std::lock_guard lock(staticResourceMutex_);

		staticResourceSlots_[handle.GetIndex()].referenceCount--;
		if (staticResourceSlots_[handle.GetIndex()].referenceCount != 0)
		{
			return;
		}

		staticResourceSlots_[handle.GetIndex()].version++;
		staticResourceSlots_[handle.GetIndex()].isFree = true;
		if (staticResourceSlots_[handle.GetIndex()].resource)
		{
			manager_->SafeRelease(std::move(staticResourceSlots_[handle.GetIndex()].resource));
		}
		freeIndices_.push(handle.GetIndex());
	}

	renderer::core::DynamicAllocation ResourceManager::CreateDynamicBuffer(size_t dataByteSize)
	{
		std::lock_guard lock(dynamicResourceMutex_);

		std::unique_ptr<renderer::resources::IGPUResource> tempBuffer =
			std::make_unique<renderer::resources::GPUBuffer>(device_, dataByteSize, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_GENERIC_READ);

		renderer::core::DynamicAllocation returnAllocation;

		returnAllocation.cpuAddress = static_cast<renderer::resources::GPUBuffer*>(tempBuffer.get())->GetMappedPtr();
		returnAllocation.gpuAddress = tempBuffer->GetGPUVirtualAddress();
		returnAllocation.offset = 0;

		dynamicBuffers_.push_back(std::move(tempBuffer));

		return returnAllocation;
	}
	void ResourceManager::ResetDynamicBuffers()
	{
		std::lock_guard lock(dynamicResourceMutex_);

		dynamicBuffers_.clear();
	}

	renderer::resources::IGPUResource* ResourceManager::GetStaticResource(renderer::core::ResourceHandle handle)
	{
		std::lock_guard lock(staticResourceMutex_);

		if (staticResourceSlots_.size() <= handle.GetIndex())
		{
			return nullptr;
		}
		if (staticResourceSlots_[handle.GetIndex()].isFree == true)
		{
			return nullptr;
		}
		if (staticResourceSlots_[handle.GetIndex()].version != handle.GetVersion())
		{
			return nullptr;
		}
		return staticResourceSlots_[handle.GetIndex()].resource.get();
	}
}

// Explicit Instantiation
namespace renderer::core
{
	template renderer::core::ResourceHandle ResourceManager::CreateStaticBuffer<renderer::command::GraphicsCommandContext>(
		renderer::command::GraphicsCommandContext& context,
		const void* data,
		size_t dataByteSize
	);
}
