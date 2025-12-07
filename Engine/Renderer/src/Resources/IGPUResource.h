#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace renderer::resources
{
	class IGPUResource
	{
	protected:
		Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
		const D3D12_HEAP_TYPE heapType_;
		const UINT64 dataByteSize_;
		D3D12_RESOURCE_STATES state_;

	public:
		IGPUResource(D3D12_HEAP_TYPE heapType, UINT64 dataByteSize, D3D12_RESOURCE_STATES initialState) :
			heapType_(heapType), dataByteSize_(dataByteSize), state_(initialState) {
		}
		virtual ~IGPUResource() = default;

		ID3D12Resource* GetResource() const { return resource_.Get(); }
		D3D12_HEAP_TYPE GetHeapType() const { return heapType_; }
		UINT64 GetByteSize() const { return dataByteSize_; }

		D3D12_RESOURCE_STATES GetState() const { return state_; }
		void SetState(D3D12_RESOURCE_STATES state) { state_ = state; }

		D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const { return resource_->GetGPUVirtualAddress(); }

	};
}
