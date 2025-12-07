#pragma once

#include <DirectXMath.h>

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// Resource related
namespace renderer::utils
{
	inline D3D12_RESOURCE_DESC1 CreateBufferResourceDesc(
		UINT64 width, D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE)
	{
		D3D12_RESOURCE_DESC1 desc;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = width;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = flags;

		return desc;
	}
	inline D3D12_RESOURCE_DESC1 CreateDepthStencil2DResourceDesc(
		UINT width, UINT height, DXGI_FORMAT format,
		bool enable4xMsaa, UINT msaaQuality)
	{
		D3D12_RESOURCE_DESC1 desc{};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = width;
		desc.Height = height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.SampleDesc.Count = enable4xMsaa ? 4 : 1;
		desc.SampleDesc.Quality = enable4xMsaa ? (msaaQuality - 1) : 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		return desc;
	}

	inline D3D12_HEAP_PROPERTIES CreateResourceHeapProps(D3D12_HEAP_TYPE type)
	{
		D3D12_HEAP_PROPERTIES props{};
		props.Type = type;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		props.CreationNodeMask = 1;
		props.VisibleNodeMask = 1;
		return props;
	}

	inline static UINT64 CalculateConstantBufferByteSize(UINT byteSize)
	{
		return (byteSize + 255) & ~255;
	}
}

// Math related
namespace renderer::utils
{
	class MathHelper
	{
	public:
		inline static DirectX::XMFLOAT4X4 Identity4x4()
		{
			static DirectX::XMFLOAT4X4 I(
				1.0f, 0.0f, 0.0f, 0.0f,
				0.0f, 1.0f, 0.0f, 0.0f,
				0.0f, 0.0f, 1.0f, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f);

			return I;
		}
	};
}
