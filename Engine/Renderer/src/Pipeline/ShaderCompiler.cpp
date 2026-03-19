#include "ShaderCompiler.h"
#include "Exception.h"

#include <stdexcept>

// Constructor
namespace renderer::pipeline
{
	ShaderCompiler::ShaderCompiler()
	{
		DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils_));
		DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler_));

		utils_->CreateDefaultIncludeHandler(&includeHandler_);
	}
}
namespace renderer::pipeline
{
	Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::Compile(const std::wstring& filename,
		const std::wstring& entryPoint,
		const std::wstring& targetProfile)
	{
		Microsoft::WRL::ComPtr<IDxcBlobEncoding> sourceBlob;
		ThrowIfFailed(utils_->LoadFile(filename.c_str(), nullptr, &sourceBlob));

		DxcBuffer sourceBuffer;
		sourceBuffer.Ptr = sourceBlob->GetBufferPointer();
		sourceBuffer.Size = sourceBlob->GetBufferSize();
		sourceBuffer.Encoding = DXC_CP_UTF8;
		//	sourceBuffer.Encoding = DXC_CP_ACP; // ANSI/UTF-8

		std::vector<LPCWSTR> arguments;
		arguments.push_back(L"-E");	// -E entry
		arguments.push_back(entryPoint.c_str());
		arguments.push_back(L"-T");	// -T profile
		arguments.push_back(targetProfile.c_str());

#if defined(_DEBUG)
		arguments.push_back(DXC_ARG_DEBUG);          // -Zi (Debug Info)
		arguments.push_back(DXC_ARG_SKIP_OPTIMIZATIONS); // -Od (No Opt)
#else
		arguments.push_back(DXC_ARG_OPTIMIZATION_LEVEL3); // -O3 (Max Opt)
#endif
		// (選擇性) 將矩陣設為 Row Major (預設是 Column Major)
		// arguments.push_back(L"-Zpr"); 

		Microsoft::WRL::ComPtr<IDxcResult> result;
		ThrowIfFailed(compiler_->Compile(
			&sourceBuffer,
			arguments.data(), (UINT32)arguments.size(),
			includeHandler_.Get(),
			IID_PPV_ARGS(&result)));

		Microsoft::WRL::ComPtr<IDxcBlobUtf8> errors;
		result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
		if (errors && errors->GetStringLength() > 0)
		{
			std::string errorMsg = (char*)errors->GetStringPointer();
			throw std::runtime_error("Shader Compile Error:\n" + errorMsg);
		}

		Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
		ThrowIfFailed(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr));

		return shaderBlob;
	}
}
