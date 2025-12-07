#pragma once

#include "DescriptorHeap.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

// forward declaration
namespace renderer::core
{
	struct DescriptorHandle;
}
namespace renderer::descriptors
{
	class DescriptorHeap;
}
namespace renderer::resources
{
	class IGPUResource;
}

// descriptorHeapManager
namespace renderer::descriptors
{
	class DescriptorHeapManager
	{
	private:
		ID3D12Device8* device_;

		renderer::descriptors::DescriptorHeap rtvHeap_;
		renderer::descriptors::DescriptorHeap dsvHeap_;
		renderer::descriptors::DescriptorHeap csuHeap_;

	public:
		DescriptorHeapManager() = delete;
		DescriptorHeapManager(ID3D12Device8* device);

	public:
		renderer::core::DescriptorHandle AllocateRTV();
		renderer::core::DescriptorHandle AllocateDSV();
		renderer::core::DescriptorHandle AllocateCSU();
		void FreeRTV(renderer::core::DescriptorHandle handle);
		void FreeDSV(renderer::core::DescriptorHandle handle);
		void FreeCSU(renderer::core::DescriptorHandle handle);

		renderer::core::DescriptorHandle CreateCBV(renderer::resources::IGPUResource* buffer);
		renderer::core::DescriptorHandle CreateCBV(D3D12_GPU_VIRTUAL_ADDRESS gpuAddress, UINT64 size);

		ID3D12DescriptorHeap* GetRTVHeap() const;
		ID3D12DescriptorHeap* GetDSVHeap() const;
		ID3D12DescriptorHeap* GetCSUHeap() const;

	};
}
