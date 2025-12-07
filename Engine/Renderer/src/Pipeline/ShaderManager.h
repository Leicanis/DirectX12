#pragma once

#include "ShaderCompiler.h"

#include <string>
#include <unordered_map>

#include <windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace renderer::pipeline
{
	class ShaderManager
	{
	private:
		std::unordered_map<std::wstring, Microsoft::WRL::ComPtr<IDxcBlob>> shaderCache_;

		ShaderCompiler compiler_;

	public:
		IDxcBlob* GetShader(const std::wstring& path, const std::wstring& entry, const std::wstring& profile);

		void ClearCache();

	};
}
