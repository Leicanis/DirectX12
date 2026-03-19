#pragma once

#include <vector>
#include <memory>
#include <string>

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace renderer::pipeline
{
	class RootSignatureBuilder
	{
	private:
		ID3D12Device8* device_;

		std::vector<D3D12_ROOT_PARAMETER1> parameters_;
		std::vector<D3D12_STATIC_SAMPLER_DESC> staticSamplers_;
		std::vector<std::unique_ptr<std::vector<D3D12_DESCRIPTOR_RANGE1>>> rangeStorage_;

	public:
		RootSignatureBuilder(ID3D12Device8* device);

	public:
		RootSignatureBuilder& AddDescriptorTable(const std::vector<D3D12_DESCRIPTOR_RANGE1>& ranges, D3D12_SHADER_VISIBILITY visiblity);
		RootSignatureBuilder& AddConstants(UINT num32BitValues, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visiblity);
		RootSignatureBuilder& AddDescriptor(D3D12_ROOT_PARAMETER_TYPE type, UINT shaderRegister, UINT registerSpace, D3D12_SHADER_VISIBILITY visiblity);
		RootSignatureBuilder& AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& sampler);

		Microsoft::WRL::ComPtr<ID3D12RootSignature> Build(D3D12_ROOT_SIGNATURE_FLAGS flags, const std::wstring& name);
		RootSignatureBuilder& Reset();

	};
}
