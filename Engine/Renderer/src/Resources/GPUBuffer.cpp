#include "GPUBuffer.h"
#include "../Utils/D3D12Helpers.h"
#include "Exception.h"

#include <cassert>

// Constructor/ Distructor
namespace renderer::resources
{
	GPUBuffer::GPUBuffer(ID3D12Device8* device, UINT64 dataByteSize, D3D12_HEAP_TYPE heapType, D3D12_RESOURCE_STATES initialState) :
		device_(device), IGPUResource(heapType, dataByteSize, initialState)
	{
		assert(device_ != nullptr && "GPUBuffer requires a valid D3D12 device pointer.");

		D3D12_HEAP_PROPERTIES bufferTypeProperties = renderer::utils::CreateResourceHeapProps(heapType_);
		D3D12_RESOURCE_DESC1 bufferTypeDesc = renderer::utils::CreateBufferResourceDesc(dataByteSize_);
		if (heapType_ == D3D12_HEAP_TYPE_UPLOAD)
		{
			state_ = D3D12_RESOURCE_STATE_GENERIC_READ;
		}
		if (heapType_ == D3D12_HEAP_TYPE_READBACK) 
		{
			state_ = D3D12_RESOURCE_STATE_COPY_DEST;
		}
		else
		{
			state_ = D3D12_RESOURCE_STATE_COMMON;
		}
		ThrowIfFailed(device_->CreateCommittedResource2(
			&bufferTypeProperties, D3D12_HEAP_FLAG_NONE,
			&bufferTypeDesc, state_,
			nullptr, nullptr, IID_PPV_ARGS(resource_.GetAddressOf())));

		D3D12_RANGE readRange{ 0, 0 };
		if (heapType_ == D3D12_HEAP_TYPE_READBACK)
		{
			ThrowIfFailed(resource_->Map(0, nullptr, &mappedData_));
		}
		else if (heapType_ == D3D12_HEAP_TYPE_UPLOAD)
		{
			ThrowIfFailed(resource_->Map(0, &readRange, &mappedData_));
		}
	}
	GPUBuffer::~GPUBuffer()
	{
		D3D12_RANGE readRange{ 0, 0 };
		if (heapType_ == D3D12_HEAP_TYPE_READBACK)
		{
			if (resource_ && mappedData_)
			{
				resource_->Unmap(0, nullptr);
			}
		}
		else if (heapType_ == D3D12_HEAP_TYPE_UPLOAD)
		{
			if (resource_ && mappedData_)
			{
				resource_->Unmap(0, &readRange);
			}
		}

		mappedData_ = nullptr;
	}
}
namespace renderer::resources
{
	void GPUBuffer::CopyData(const void* data, size_t size, size_t offset = 0)
	{
		assert(heapType_ == D3D12_HEAP_TYPE_UPLOAD && "Only Upload Buffers can be written to by CPU!");
		assert(size <= dataByteSize_ && "Data size is larger than buffer size!");
		assert(mappedData_ != nullptr && "Buffer is not mapped!");
		assert(state_ != D3D12_RESOURCE_STATE_COPY_DEST && "Buffer state error.");

		uint8_t* destPtr = static_cast<uint8_t*>(mappedData_) + offset;
		memcpy(destPtr, data, size);
	}
	void GPUBuffer::ReadData(void* dest, size_t size) 
	{
		assert(heapType_ == D3D12_HEAP_TYPE_READBACK && "Read only allowed on Readback Buffer");
		assert(size <= dataByteSize_ && "Data size is larger than buffer size!");
		assert(mappedData_ != nullptr && "Buffer is not mapped!");
		assert(state_ != D3D12_RESOURCE_STATE_COPY_SOURCE && "Buffer state error.");

		memcpy(dest, mappedData_, size);
	}

	D3D12_VERTEX_BUFFER_VIEW GPUBuffer::GetVertexBufferView(UINT stride) const
	{
		D3D12_VERTEX_BUFFER_VIEW vbv = {};
		vbv.BufferLocation = resource_->GetGPUVirtualAddress();
		vbv.SizeInBytes = static_cast<UINT>(dataByteSize_);
		vbv.StrideInBytes = stride;

		return vbv;
	}
	D3D12_INDEX_BUFFER_VIEW GPUBuffer::GetIndexBufferView(DXGI_FORMAT format = DXGI_FORMAT_R32_UINT) const
	{
		D3D12_INDEX_BUFFER_VIEW ibv = {};
		ibv.BufferLocation = resource_->GetGPUVirtualAddress();
		ibv.SizeInBytes = static_cast<UINT>(dataByteSize_);
		ibv.Format = format;

		return ibv;
	}

	void* GPUBuffer::GetMappedPtr() const
	{
		return mappedData_;
	}
}
