#pragma once

#include "../Resources/IGPUResource.h"

#include <mutex>
#include <memory>
#include <queue>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// Forward declaration
namespace renderer::core
{
	enum class CommandType;
}

namespace renderer::command
{
	class CommandQueueManager
	{
	private:
		struct CommandQueueResource
		{
			Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
			Microsoft::WRL::ComPtr<ID3D12Fence1> fence;
			UINT64 nextFenceValue = 1;

			std::mutex mutex;
		};

		struct StaleResourceWrapper
		{
			std::unique_ptr<renderer::resources::IGPUResource> resource = nullptr;
			uint64_t graphicsFence = 0;
			uint64_t computeFence = 0;
			uint64_t copyFence = 0;
		};
		struct StaleObjectWrapper
		{
			Microsoft::WRL::ComPtr<ID3D12DeviceChild> object = nullptr;
			uint64_t graphicsFence = 0;
			uint64_t computeFence = 0;
			uint64_t copyFence = 0;
		};

	private:
		ID3D12Device8* device_;

		CommandQueueResource graphicsQueueResource_;
		CommandQueueResource computeQueueResource_;
		CommandQueueResource copyQueueResource_;

		std::queue<StaleResourceWrapper> staleResourcesQueue_;
		std::queue<StaleObjectWrapper> staleObjectsQueue_;
		std::mutex staleResourcesMutex_;
		std::mutex staleObjectsMutex_;

	private:
		void CreateCommandQueueResourceInternal(CommandQueueResource& commandQueueResource, D3D12_COMMAND_LIST_TYPE type);
		CommandQueueResource* GetQueueResourcesPtrInternal(renderer::core::CommandType type);

	public:
		CommandQueueManager() = delete;
		CommandQueueManager(ID3D12Device8* device);

	public:
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE type);
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> CreateCommandList(ID3D12CommandAllocator* commandAllocator, D3D12_COMMAND_LIST_TYPE type);

		uint64_t SubmitCommandListAndSignal(renderer::core::CommandType type, ID3D12CommandList* commandList);
		
		void WaitForFence(renderer::core::CommandType type, UINT64 fenceValue);
		void WaitForGPU(renderer::core::CommandType type);

		uint64_t GetFenceValue(renderer::core::CommandType type);
		ID3D12CommandQueue* GetCommandQueue(renderer::core::CommandType type);

		void SafeRelease(std::unique_ptr<renderer::resources::IGPUResource> resource);
		void SafeRelease(Microsoft::WRL::ComPtr<ID3D12DeviceChild> object);
		void ProcessDeferredReleases();

	};
}
