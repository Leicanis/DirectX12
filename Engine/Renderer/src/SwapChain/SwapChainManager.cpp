#include "SwapChainManager.h"
#include "../Command/CommandQueueManager.h"
#include "../Descriptors/DescriptorHeapManager.h"
#include "../Resources/TextureResource.h"
#include "../Utils/D3D12Helpers.h"
#include "../Core/RendererType.h"
#include "Exception.h"

// Constructor
namespace renderer::display
{
	SwapChainManager::SwapChainManager(
		ID3D12Device8* device, IDXGIFactory6* factory, ID3D12CommandQueue* commandQueue,
		renderer::command::CommandQueueManager* commandQueueManager,
		renderer::descriptors::DescriptorHeapManager* descriptorManager,
		HWND hwnd, UINT width, UINT height) :
		device_(device), factory_(factory), 
		commandQueue_(commandQueue),
		commandQueueManager_(commandQueueManager), 
		descriptorManager_(descriptorManager), 
		hwnd_(hwnd), width_(width), height_(height)
	{
		assert(device_ != nullptr && "SwapChainManager requires a valid D3D12 device pointer.");
		assert(factory_ != nullptr && "SwapChainManager requires a valid D3D12 factory pointer.");
		assert(commandQueue_ != nullptr && "SwapChainManager requires a valid D3D12 CommandQueue pointer.");
		assert(commandQueueManager_ != nullptr && "SwapChainManager requires a valid CommandQueurManager pointer.");
		assert(descriptorManager_ != nullptr && "SwapChainManager requires a valid DescriptorManager pointer.");

		CreateSwapChainInternal();
		CreateDSVInternal();
	}
}
// Internal method
namespace renderer::display
{
	void SwapChainManager::CreateSwapChainInternal()
	{
		for (int i = 0; i < BufferCount; i++)
		{
			rtvHandles_[i] = descriptorManager_->AllocateRTV();
		}

		// 創建 swapChain
		DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc1{};
		dxgiSwapChainDesc1.BufferCount = BufferCount;
		dxgiSwapChainDesc1.Width = width_;
		dxgiSwapChainDesc1.Height = height_;
		dxgiSwapChainDesc1.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgiSwapChainDesc1.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgiSwapChainDesc1.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		dxgiSwapChainDesc1.SampleDesc.Count = 1;
		ThrowIfFailed(factory_->CreateSwapChainForHwnd(
			commandQueue_, hwnd_, &dxgiSwapChainDesc1, nullptr, nullptr,
			reinterpret_cast<IDXGISwapChain1**>(dxgiSwapChain_.GetAddressOf())));

		// 個別創建 RTV resource view
		for (UINT i = 0; i < BufferCount; i++)
		{
			ThrowIfFailed(dxgiSwapChain_->GetBuffer(i, IID_PPV_ARGS(backBuffers_[i].GetAddressOf())));

			device_->CreateRenderTargetView(
				backBuffers_[i].Get(), nullptr, rtvHandles_[i].cpuHandle);
		}
	}
	void SwapChainManager::CreateDSVInternal()
	{
		dsvHandle_ = descriptorManager_->AllocateDSV();
		dsvTexture_ = std::make_unique<renderer::resources::Texture>(
			device_, width_, height_, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT);

		// 創建 DSV resource view
		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc{};
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		device_->CreateDepthStencilView(
			dsvTexture_->GetResource(), &dsvDesc, dsvHandle_.cpuHandle);
	}
}
namespace renderer::display
{
	void SwapChainManager::Clear(renderer::command::GraphicsCommandContext& commandContext, float clearColor[])
	{
		ID3D12Resource* backBuffer = backBuffers_[dxgiSwapChain_->GetCurrentBackBufferIndex()].Get();
		commandContext.TransitionResource(backBuffer, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		renderer::core::DescriptorHandle rtvHandle = rtvHandles_[dxgiSwapChain_->GetCurrentBackBufferIndex()];
		renderer::core::DescriptorHandle dsvHandle = dsvHandle_;
		commandContext.SetRenderTargets(1, &rtvHandle, &dsvHandle);

		commandContext.ClearRenderTarget(rtvHandle, clearColor);
		commandContext.ClearDepthStencil(dsvHandle);
	}
	void SwapChainManager::PreparePresent(renderer::command::GraphicsCommandContext& commandContext)
	{
		ID3D12Resource* backBuffer = backBuffers_[dxgiSwapChain_->GetCurrentBackBufferIndex()].Get();
		commandContext.TransitionResource(backBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
	}
	void SwapChainManager::Present()
	{
		ThrowIfFailed(dxgiSwapChain_->Present(1, 0));
	}

	void SwapChainManager::Resize(UINT width, UINT height)
	{
		if (width == 0 || height == 0) return;
		width_ = width;
		height_ = height;

		commandQueueManager_->WaitForGPU(renderer::core::CommandType::Graphics);

		for (int i = 0; i < BufferCount; ++i)
		{
			backBuffers_[i].Reset();
		}
		dsvTexture_.reset();

		DXGI_SWAP_CHAIN_DESC1 desc = {};
		dxgiSwapChain_->GetDesc1(&desc);
		ThrowIfFailed(dxgiSwapChain_->ResizeBuffers(
			BufferCount,
			width, height,
			desc.Format,
			desc.Flags));

		for (UINT i = 0; i < BufferCount; i++)
		{
			ThrowIfFailed(dxgiSwapChain_->GetBuffer(i, IID_PPV_ARGS(backBuffers_[i].GetAddressOf())));

			device_->CreateRenderTargetView(
				backBuffers_[i].Get(),
				nullptr,
				rtvHandles_[i].cpuHandle);
		}

		dsvTexture_ = std::make_unique<renderer::resources::Texture>(
			device_, width_, height_, DXGI_FORMAT_R24G8_TYPELESS, DXGI_FORMAT_D24_UNORM_S8_UINT);

		D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};
		dsvDesc.Flags = D3D12_DSV_FLAG_NONE;
		dsvDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		dsvDesc.Texture2D.MipSlice = 0;
		device_->CreateDepthStencilView(
			dsvTexture_->GetResource(), &dsvDesc, dsvHandle_.cpuHandle);
	}
}
