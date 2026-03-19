#pragma once

#include <string>
#include <unordered_map>

#include <windows.h>
#include <d3d12.h>
#include <dxcapi.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace renderer::pipeline
{
	class ShaderCompiler
	{
	private:
		Microsoft::WRL::ComPtr<IDxcUtils> utils_;
		Microsoft::WRL::ComPtr<IDxcCompiler3> compiler_;
		Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_;

	public:
		ShaderCompiler();

		Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::wstring& filename,
			const std::wstring& entryPoint,
			const std::wstring& targetProfile);

	};
}
