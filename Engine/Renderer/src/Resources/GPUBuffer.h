#pragma once

#include "IGPUResource.h"

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// Forward declaration
namespace renderer::resources
{
	class IGPUResource;
}

namespace renderer::resources
{
	class GPUBuffer : public IGPUResource
	{
	private:
		ID3D12Device8* device_;

		void* mappedData_ = nullptr;

	public:
		GPUBuffer(ID3D12Device8* device, UINT64 dataByteSize, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState);
		~GPUBuffer();

	public:
		void CopyData(const void* data, size_t size, size_t offset);
		void ReadData(void* dest, size_t size);

		D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView(UINT stride) const;
		D3D12_INDEX_BUFFER_VIEW GetIndexBufferView(DXGI_FORMAT format) const;

		void* GetMappedPtr() const;
	};
}
