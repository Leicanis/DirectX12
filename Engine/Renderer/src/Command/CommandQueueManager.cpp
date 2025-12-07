#include "CommandQueueManager.h"
#include "../Core/RendererType.h"
#include "Exception.h"

#include <cassert>


namespace renderer::command
{
	void CommandQueueManager::CreateCommandQueueResourceInternal(CommandQueueResource& commandQueueResource, D3D12_COMMAND_LIST_TYPE type)
	{
		D3D12_COMMAND_QUEUE_DESC commandQueueDesc{ type, D3D12_COMMAND_QUEUE_FLAG_NONE };
		ThrowIfFailed(device_->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(commandQueueResource.commandQueue.GetAddressOf())));
		ThrowIfFailed(device_->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(commandQueueResource.fence.GetAddressOf())));
	}
	CommandQueueManager::CommandQueueResource* CommandQueueManager::GetQueueResourcesPtrInternal(renderer::core::CommandType type)
	{
		switch (type)
		{
		case renderer::core::CommandType::Graphics:	return &graphicsQueueResource_;
		case renderer::core::CommandType::Compute:	return &computeQueueResource_;
		case renderer::core::CommandType::Copy:		return &copyQueueResource_;
		default:
			assert(false && "Invalid ContextType in GetQueueResourcesPtrInternal.");
			return nullptr;
		}
	}
}
//Constructor
namespace renderer::command
{
	CommandQueueManager::CommandQueueManager(ID3D12Device8* device) : device_(device)
	{
		assert(device != nullptr && "CommandManager needs a available ID3D12Device8 pointer.");

		CreateCommandQueueResourceInternal(graphicsQueueResource_, D3D12_COMMAND_LIST_TYPE_DIRECT);
		CreateCommandQueueResourceInternal(computeQueueResource_, D3D12_COMMAND_LIST_TYPE_COMPUTE);
		CreateCommandQueueResourceInternal(copyQueueResource_, D3D12_COMMAND_LIST_TYPE_COPY);
	}
}
namespace renderer::command
{
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CommandQueueManager::CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type)
	{
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> tempAllocator;
		ThrowIfFailed(device_->CreateCommandAllocator(type, IID_PPV_ARGS(tempAllocator.GetAddressOf())));

		return tempAllocator;
	}
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CommandQueueManager::CreateCommandList(ID3D12CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type)
	{
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> tempList;
		ThrowIfFailed(device_->CreateCommandList(0, type, commandAllocator, nullptr, IID_PPV_ARGS(tempList.GetAddressOf())));

		return tempList;
	}

	uint64_t CommandQueueManager::SubmitCommandListAndSignal(renderer::core::CommandType type, ID3D12CommandList* commandList)
	{
		CommandQueueResource* commandQueueResource = GetQueueResourcesPtrInternal(type);
		ID3D12CommandQueue* commandQueue = commandQueueResource->commandQueue.Get();
		ID3D12CommandList* lists[] = { commandList };

		std::lock_guard lock(commandQueueResource->mutex);
		commandQueue->ExecuteCommandLists(1, lists);

		uint64_t fenceValue = GetQueueResourcesPtrInternal(type)->nextFenceValue++;
		ThrowIfFailed(commandQueue->Signal(commandQueueResource->fence.Get(), fenceValue));

		return fenceValue;
	}

	void CommandQueueManager::WaitForFence(renderer::core::CommandType type, UINT64 fenceValue)
	{
		CommandQueueResource* commandQueueResource = GetQueueResourcesPtrInternal(type);

		if (commandQueueResource->fence->GetCompletedValue() >= fenceValue)
		{
			return;
		}
		HANDLE eventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		assert(eventHandle != nullptr && "WaitForFence: Create Event failed.");
		ThrowIfFailed(commandQueueResource->fence->SetEventOnCompletion(fenceValue, eventHandle));

		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}
	void CommandQueueManager::WaitForGPU(renderer::core::CommandType type)
	{
		CommandQueueResource* commandQueueResource = GetQueueResourcesPtrInternal(type);

		if (commandQueueResource->fence->GetCompletedValue() >= commandQueueResource->nextFenceValue)
		{
			return;
		}

		UINT64 fenceValueToSignal = 0;
		{
			std::lock_guard lock(commandQueueResource->mutex);
			fenceValueToSignal = commandQueueResource->nextFenceValue++;
			ThrowIfFailed(commandQueueResource->commandQueue->Signal(commandQueueResource->fence.Get(), fenceValueToSignal));
		}

		WaitForFence(type, fenceValueToSignal);
	}

	uint64_t CommandQueueManager::GetFenceValue(renderer::core::CommandType type)
	{
		return GetQueueResourcesPtrInternal(type)->fence->GetCompletedValue();
	}
	ID3D12CommandQueue* CommandQueueManager::GetCommandQueue(renderer::core::CommandType type)
	{
		switch (type)
		{
		case renderer::core::CommandType::Graphics: return graphicsQueueResource_.commandQueue.Get();
		case renderer::core::CommandType::Compute:	return computeQueueResource_.commandQueue.Get();
		case renderer::core::CommandType::Copy:		return copyQueueResource_.commandQueue.Get();
		default:
			assert(false && "Invalid CommandType in GetCommandQueue.");
			return nullptr;
		}
	}

	void CommandQueueManager::SafeRelease(std::unique_ptr<renderer::resources::IGPUResource> resource)
	{
		if (resource == nullptr)
		{
			return;
		}

		std::lock_guard lock(staleResourcesMutex_);

		uint64_t currentGraphicsQueueFence = graphicsQueueResource_.nextFenceValue;
		uint64_t currentComputeQueueFence = computeQueueResource_.nextFenceValue;
		uint64_t currentCopyQueueFence = copyQueueResource_.nextFenceValue;
		staleResourcesQueue_.push({ std::move(resource), currentGraphicsQueueFence, currentComputeQueueFence, currentCopyQueueFence });
	}
	void CommandQueueManager::SafeRelease(Microsoft::WRL::ComPtr<ID3D12DeviceChild> object)
	{
		if (object == nullptr)
		{
			return;
		}

		std::lock_guard lock(staleObjectsMutex_);

		uint64_t currentGraphicsQueueFence = graphicsQueueResource_.nextFenceValue;
		uint64_t currentComputeQueueFence = computeQueueResource_.nextFenceValue;
		uint64_t currentCopyQueueFence = copyQueueResource_.nextFenceValue;
		staleObjectsQueue_.push({ std::move(object), currentGraphicsQueueFence, currentComputeQueueFence, currentCopyQueueFence });
	}
	void CommandQueueManager::ProcessDeferredReleases()
	{
		std::lock_guard resourceLock(staleResourcesMutex_);
		std::lock_guard objectLock(staleObjectsMutex_);

		UINT64 completedGraphicsQueueFence = graphicsQueueResource_.fence->GetCompletedValue();
		UINT64 completedComputeQueueFence = graphicsQueueResource_.fence->GetCompletedValue();
		UINT64 completedCopyQueueFence = graphicsQueueResource_.fence->GetCompletedValue();

		while (!staleResourcesQueue_.empty())
		{
			if (staleResourcesQueue_.front().graphicsFence <= completedGraphicsQueueFence &&
				staleResourcesQueue_.front().computeFence <= completedComputeQueueFence &&
				staleResourcesQueue_.front().copyFence <= completedCopyQueueFence)
			{
				staleResourcesQueue_.pop();
			}
			else
			{
				break;
			}
		}
		while (!staleObjectsQueue_.empty())
		{
			if (staleObjectsQueue_.front().graphicsFence <= completedGraphicsQueueFence &&
				staleObjectsQueue_.front().computeFence <= completedComputeQueueFence &&
				staleObjectsQueue_.front().copyFence <= completedCopyQueueFence)
			{
				staleObjectsQueue_.pop();
			}
			else
			{
				break;
			}
		}
	}
}
