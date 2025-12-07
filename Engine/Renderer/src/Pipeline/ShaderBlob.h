#pragma once

#include "../Core/RendererType.h"

#include <windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <wrl.h>


namespace renderer::pipeline
{
	struct ShaderBlob
	{
		Microsoft::WRL::ComPtr<IDxcBlob> blob;
		renderer::ShaderType type;

		D3D12_SHADER_BYTECODE GetByteCode()
		{
			return { blob->GetBufferPointer(), blob->GetBufferSize() };
		}
	};
}
