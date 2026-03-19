#pragma once

#include "../Command/CommandContext.h"
#include "../Resources/TextureResource.h"

#include <memory>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// forward declaration
namespace renderer::command
{
	class CommandQueueManager;
}
namespace renderer::descriptors
{
	class DescriptorHeapManager;
}

namespace renderer::display
{
	class SwapChainManager
	{
	public:
		static const UINT BufferCount = 2U;

	private:
		ID3D12Device8* device_;
		IDXGIFactory6* factory_;
		ID3D12CommandQueue* commandQueue_;
		renderer::command::CommandQueueManager* commandQueueManager_;
		renderer::descriptors::DescriptorHeapManager* descriptorManager_;

		HWND hwnd_;
		UINT width_, height_;
		Microsoft::WRL::ComPtr<IDXGISwapChain4> dxgiSwapChain_;

		Microsoft::WRL::ComPtr<ID3D12Resource> backBuffers_[BufferCount];
		renderer::core::DescriptorHandle rtvHandles_[BufferCount]{};

		std::unique_ptr<renderer::resources::Texture> dsvTexture_ = nullptr;
		renderer::core::DescriptorHandle dsvHandle_{};

	private:
		void CreateSwapChainInternal();
		void CreateDSVInternal();

	public:
		SwapChainManager() = delete;
		SwapChainManager(
			ID3D12Device8* device, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue,
			renderer::command::CommandQueueManager* commandQueueManager,
			renderer::descriptors::DescriptorHeapManager* descriptorManager, 
			HWND hwnd, UINT width, UINT height);

	public:
		void Clear(renderer::command::GraphicsCommandContext& commandContext, float clearColor[]);
		void PreparePresent(renderer::command::GraphicsCommandContext& commandContext);
		void Present();

		void Resize(UINT width, UINT height);

		ID3D12Resource* GetCurrentBackBuffer() const { return backBuffers_[dxgiSwapChain_->GetCurrentBackBufferIndex()].Get(); }
		renderer::core::DescriptorHandle GetCurrentRTV() const { return rtvHandles_[dxgiSwapChain_->GetCurrentBackBufferIndex()]; }
		renderer::core::DescriptorHandle GetDSV() const { return dsvHandle_; }

	};
}
