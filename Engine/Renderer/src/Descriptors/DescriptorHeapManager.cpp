#include "DescriptorHeapManager.h"
#include "../Core/RendererType.h"
#include "../Resources/IGPUResource.h"
#include"../Utils/D3D12Helpers.h"

#include <utility>
#include <cassert>

// Constructor
namespace renderer::descriptors
{
	DescriptorHeapManager::DescriptorHeapManager(ID3D12Device8* device) : 
		device_(device),
		rtvHeap_(device_, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 128, false),
		dsvHeap_(device_, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 128, false),
		csuHeap_(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096, true)
	{
		assert(device_ != nullptr && "DescriptorHeapManager requires a valid D3D12 device pointer.");
	}
}
namespace renderer::descriptors
{
	renderer::core::DescriptorHandle DescriptorHeapManager::AllocateRTV()
	{
		return rtvHeap_.AllocateNewDescriptor();
	}
	renderer::core::DescriptorHandle DescriptorHeapManager::AllocateDSV()
	{
		return dsvHeap_.AllocateNewDescriptor();
	}
	renderer::core::DescriptorHandle DescriptorHeapManager::AllocateCSU()
	{
		return csuHeap_.AllocateNewDescriptor();
	}
	void DescriptorHeapManager::FreeRTV(renderer::core::DescriptorHandle handle)
	{
		rtvHeap_.FreeDescriptor(handle);
	}
	void DescriptorHeapManager::FreeDSV(renderer::core::DescriptorHandle handle)
	{
		dsvHeap_.FreeDescriptor(handle);
	}
	void DescriptorHeapManager::FreeCSU(renderer::core::DescriptorHandle handle)
	{
		csuHeap_.FreeDescriptor(handle);
	}

	renderer::core::DescriptorHandle DescriptorHeapManager::CreateCBV(renderer::resources::IGPUResource* buffer)
	{
		assert(buffer != nullptr);

		renderer::core::DescriptorHandle handle = AllocateCSU();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = buffer->GetGPUVirtualAddress();
		cbvDesc.SizeInBytes = static_cast<UINT>(renderer::utils::CalculateConstantBufferByteSize(buffer->GetByteSize()));

		device_->CreateConstantBufferView(&cbvDesc, handle.cpuHandle);

		return handle;
	}
	renderer::core::DescriptorHandle DescriptorHeapManager::CreateCBV(D3D12_GPU_VIRTUAL_ADDRESS gpuAddress, UINT64 size)
	{
		renderer::core::DescriptorHandle handle = AllocateCSU();

		D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
		cbvDesc.BufferLocation = gpuAddress;
		cbvDesc.SizeInBytes = static_cast<UINT>(renderer::utils::CalculateConstantBufferByteSize(size));

		device_->CreateConstantBufferView(&cbvDesc, handle.cpuHandle);

		return handle;
	}

	ID3D12DescriptorHeap* DescriptorHeapManager::GetRTVHeap() const { return rtvHeap_.GetDescriptorHeap(); }
	ID3D12DescriptorHeap* DescriptorHeapManager::GetDSVHeap() const { return dsvHeap_.GetDescriptorHeap(); }
	ID3D12DescriptorHeap* DescriptorHeapManager::GetCSUHeap() const { return csuHeap_.GetDescriptorHeap(); }
}
