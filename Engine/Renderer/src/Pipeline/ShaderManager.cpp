#include "ShaderManager.h"

namespace renderer::pipeline
{
	IDxcBlob* ShaderManager::GetShader(const std::wstring& path, const std::wstring& entry, const std::wstring& profile)
	{
		std::wstring key = path + L"_" + entry + L"_" + profile;

		auto it = shaderCache_.find(key);
		if (it != shaderCache_.end())
		{
			return it->second.Get();
		}

		auto blob = compiler_.Compile(path, entry, profile);

		shaderCache_[key] = blob;
		return blob.Get();
	}

	void ShaderManager::ClearCache() { shaderCache_.clear(); }
}
