#include "DescriptorHeap.h"
#include "../Core/RendererType.h"
#include "Exception.h"

#include <cassert>

// Constructor
namespace renderer::descriptors
{
	DescriptorHeap::DescriptorHeap(ID3D12Device8* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool isShaderVisible = false) :
		device_(device), heapType_(heapType), numDescriptors_(numDescriptors), descriptorSize_(0)
	{
		assert(device_ != nullptr && "DescriptorHeap requires a valid D3D12 device pointer.");

		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = heapType_;
		desc.NumDescriptors = numDescriptors;
		desc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

		if (heapType == D3D12_DESCRIPTOR_HEAP_TYPE_RTV || heapType == D3D12_DESCRIPTOR_HEAP_TYPE_DSV)
		{
			desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		}

		ThrowIfFailed(device_->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap_)));
		descriptorSize_ = device->GetDescriptorHandleIncrementSize(heapType_);
	}
}
namespace renderer::descriptors
{
	renderer::core::DescriptorHandle DescriptorHeap::AllocateNewDescriptor()
	{
		std::lock_guard lock(heapMutex_);

		uint64_t index = 0;

		if (!freeIndices_.empty())
		{
			index = freeIndices_.front();
			freeIndices_.pop();
		}
		else
		{
			assert(currentOffset_ < numDescriptors_ && "AllocateNewDescriptor error: Heap full!");

			index = currentOffset_++;
		}
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{ 0 };
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{ 0 };

		cpuHandle = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
		cpuHandle.ptr += index * descriptorSize_;
		if (descriptorHeap_->GetDesc().Flags & D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE)
		{
			gpuHandle = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
			gpuHandle.ptr += index * descriptorSize_;
		}
		return renderer::core::DescriptorHandle{ cpuHandle, gpuHandle, index };
	}
	void DescriptorHeap::FreeDescriptor(renderer::core::DescriptorHandle handle)
	{
		std::lock_guard lock(heapMutex_);

		freeIndices_.push(handle.index);
	}
}
