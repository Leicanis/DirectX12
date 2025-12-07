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
	class Texture : public IGPUResource
	{
	private:
		ID3D12Device8* device_;

	public:
		Texture(ID3D12Device8* device, UINT width, UINT height, DXGI_FORMAT resourceFormat, DXGI_FORMAT clearValueFormat);

	};
}
