#pragma once

#include <queue>
#include <mutex>

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// Forward declaration
namespace renderer::core
{
	struct DescriptorHandle;
}

namespace renderer::descriptors
{
	class DescriptorHeap
	{
	private:
		ID3D12Device8* device_;

		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap_;
		D3D12_DESCRIPTOR_HEAP_TYPE heapType_;
		UINT descriptorSize_;
		UINT numDescriptors_;

		UINT currentOffset_ = 0;
		std::queue<uint64_t> freeIndices_;

		std::mutex heapMutex_;

	public:
		DescriptorHeap() = delete;
		DescriptorHeap(ID3D12Device8* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, UINT numDescriptors, bool isShaderVisible);

	public:
		renderer::core::DescriptorHandle AllocateNewDescriptor();
		void FreeDescriptor(renderer::core::DescriptorHandle handle);

		ID3D12DescriptorHeap* GetDescriptorHeap() const { return descriptorHeap_.Get(); }

	};
}
