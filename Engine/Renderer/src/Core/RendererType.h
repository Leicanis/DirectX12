#pragma once

#include <cstdint>
#include <d3d12.h>

// CommandType
namespace renderer::core
{
	enum class CommandType { Graphics, Compute, Copy };
}
// ResourceHandle
namespace renderer::core
{
	struct ResourceHandle
	{
		uint64_t handle = 0;

		uint32_t GetIndex() const { return handle & 0xFFFFFFFF; }
		uint32_t GetVersion() const { return handle >> 32; }

		bool operator==(const ResourceHandle& other) const { return handle == other.handle; }
		bool IsValid() const { return handle != 0; }
	};

	struct DynamicAllocation
	{
		void* cpuAddress = nullptr;
		D3D12_GPU_VIRTUAL_ADDRESS gpuAddress = 0;
		size_t offset = 0;

		bool IsValid() const { return cpuAddress != nullptr; }
	};
}
// DescriptorHandle
namespace renderer::core
{
	struct DescriptorHandle
	{
		D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
		uint64_t index;
	};
}
// ShaderType
namespace renderer
{
	enum class ShaderType
	{
		VertexShader, PixelShader, ComputeShader
	};
}
