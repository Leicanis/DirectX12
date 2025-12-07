#include "TextureResource.h"
#include "../Utils/D3D12Helpers.h"
#include "Exception.h"

#include <cassert>

// Constructor
namespace renderer::resources
{
	Texture::Texture(ID3D12Device8* device, UINT width, UINT height, DXGI_FORMAT resourceFormat, DXGI_FORMAT clearValueFormat)
		: device_(device), IGPUResource(D3D12_HEAP_TYPE_DEFAULT, 0, D3D12_RESOURCE_STATE_DEPTH_WRITE)
	{
		D3D12_HEAP_PROPERTIES heapProps = renderer::utils::CreateResourceHeapProps(D3D12_HEAP_TYPE_DEFAULT);
		D3D12_RESOURCE_DESC1 desc = renderer::utils::CreateDepthStencil2DResourceDesc(
			width, height, resourceFormat, false, 0);

		D3D12_CLEAR_VALUE optClear = {};
		optClear.Format = clearValueFormat;
		optClear.DepthStencil.Depth = 1.0f;
		optClear.DepthStencil.Stencil = 0;

		ThrowIfFailed(device->CreateCommittedResource2(
			&heapProps, D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&optClear,
			nullptr,
			IID_PPV_ARGS(resource_.GetAddressOf()))
		);
	}
}
