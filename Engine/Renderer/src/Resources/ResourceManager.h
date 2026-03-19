#pragma once

#include "IGPUResource.h"
#include "../Core/RendererType.h"

#include <vector>
#include <queue>
#include <memory>
#include <mutex>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// TODO: Replace with Ring Buffer
// Forward declaration
namespace renderer::command
{
	class CommandQueueManager;
}

namespace renderer::core
{
	class ResourceManager
	{
	private:
		struct ResourceSlot
		{
			std::unique_ptr<renderer::resources::IGPUResource> resource = nullptr;
			uint32_t version = 1;
			std::atomic<uint32_t> referenceCount = 0;
			bool isFree = true;

			ResourceSlot(std::unique_ptr<renderer::resources::IGPUResource> resource, uint32_t version, bool is_free)
				: resource(std::move(resource)), version(version), referenceCount(0), isFree(is_free) {
			}

			ResourceSlot(const ResourceSlot&) = delete;
			ResourceSlot& operator=(const ResourceSlot&) = delete;

			ResourceSlot(ResourceSlot&& other) noexcept :
				resource(std::move(other.resource)), version(other.version), referenceCount(other.referenceCount.load()), isFree(other.isFree) {
			}
			ResourceSlot& operator=(ResourceSlot&& other) noexcept
			{
				if (this != &other)
				{
					resource = std::move(other.resource);
					version = other.version;
					referenceCount.store(other.referenceCount.load());
					isFree = other.isFree;
				}
				return *this;
			}
		};

	private:
		ID3D12Device8* device_;
		renderer::command::CommandQueueManager* manager_;

		std::vector<ResourceSlot> staticResourceSlots_{};
		std::queue<uint32_t> freeIndices_{};
		std::mutex staticResourceMutex_;

		std::vector<std::unique_ptr<renderer::resources::IGPUResource>> dynamicBuffers_{};
		std::mutex dynamicResourceMutex_;

	private:
		renderer::core::ResourceHandle GetStaticResourceHandleInternal();

	public:
		ResourceManager() = delete;
		ResourceManager(ID3D12Device8* device, renderer::command::CommandQueueManager* manager);
		~ResourceManager();

	public:
		template <typename TCommandContext>
		renderer::core::ResourceHandle CreateStaticBuffer(
			TCommandContext& context, const void* data, size_t dataByteSize);
		void DestroyStaticBuffer(renderer::core::ResourceHandle handle);

		renderer::core::DynamicAllocation CreateDynamicBuffer(size_t dataByteSize);
		void ResetDynamicBuffers();

		renderer::resources::IGPUResource* GetStaticResource(renderer::core::ResourceHandle handle);

	};
}
