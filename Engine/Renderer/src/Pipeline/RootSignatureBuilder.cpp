#include "RootSignatureBuilder.h"
#include "Exception.h"

#include <cassert>
#include <stdexcept>

// Constructor
namespace renderer::pipeline
{
	RootSignatureBuilder::RootSignatureBuilder(ID3D12Device8* device) : device_(device)
	{
		assert(device_ != nullptr && "RootSignatureBuilder requires a valid D3D12 device pointer.");
	}
}
namespace renderer::pipeline
{
	RootSignatureBuilder& RootSignatureBuilder::AddDescriptorTable(
		const std::vector<D3D12_DESCRIPTOR_RANGE1>& ranges, D3D12_SHADER_VISIBILITY visiblity = D3D12_SHADER_VISIBILITY_ALL)
	{
		std::unique_ptr<std::vector<D3D12_DESCRIPTOR_RANGE1>> rangeCopy = std::make_unique<std::vector<D3D12_DESCRIPTOR_RANGE1>>(ranges);
		const D3D12_DESCRIPTOR_RANGE1* rangePtr = rangeCopy->data();
		rangeStorage_.push_back(std::move(rangeCopy));

		D3D12_ROOT_PARAMETER1 tempParameter{};
		tempParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		tempParameter.ShaderVisibility = visiblity;
		tempParameter.DescriptorTable.NumDescriptorRanges = (UINT)ranges.size();
		tempParameter.DescriptorTable.pDescriptorRanges = rangePtr;

		parameters_.push_back(tempParameter);

		return *this;
	}
	RootSignatureBuilder& RootSignatureBuilder::AddConstants(
		UINT num32BitValues, UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visiblity = D3D12_SHADER_VISIBILITY_ALL)
	{
		D3D12_ROOT_PARAMETER1 tempParameter{};
		tempParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		tempParameter.ShaderVisibility = visiblity;
		tempParameter.Constants.Num32BitValues = num32BitValues;
		tempParameter.Constants.ShaderRegister = shaderRegister;
		tempParameter.Constants.RegisterSpace = registerSpace;

		parameters_.push_back(tempParameter);

		return *this;
	}
	RootSignatureBuilder& RootSignatureBuilder::AddDescriptor(
		D3D12_ROOT_PARAMETER_TYPE type, UINT shaderRegister, UINT registerSpace = 0, D3D12_SHADER_VISIBILITY visiblity = D3D12_SHADER_VISIBILITY_ALL)
	{
		D3D12_ROOT_PARAMETER1 tempParameter{};
		tempParameter.ParameterType = type;
		tempParameter.ShaderVisibility = visiblity;
		tempParameter.Descriptor.ShaderRegister = shaderRegister;
		tempParameter.Descriptor.RegisterSpace = registerSpace;

		parameters_.push_back(tempParameter);

		return *this;
	}
	RootSignatureBuilder& RootSignatureBuilder::AddStaticSampler(const D3D12_STATIC_SAMPLER_DESC& sampler)
	{
		staticSamplers_.push_back(sampler);

		return *this;
	}

	Microsoft::WRL::ComPtr<ID3D12RootSignature> RootSignatureBuilder::Build(D3D12_ROOT_SIGNATURE_FLAGS flags, const std::wstring& name)
	{
		D3D12_VERSIONED_ROOT_SIGNATURE_DESC versionedDesc{};
		versionedDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
		versionedDesc.Desc_1_1.NumParameters = (UINT)parameters_.size();
		versionedDesc.Desc_1_1.pParameters = parameters_.data();
		versionedDesc.Desc_1_1.NumStaticSamplers = (UINT)staticSamplers_.size();
		versionedDesc.Desc_1_1.pStaticSamplers = staticSamplers_.data();
		versionedDesc.Desc_1_1.Flags = flags;

		Microsoft::WRL::ComPtr<ID3DBlob> serializedRootSignature = nullptr;
		Microsoft::WRL::ComPtr<ID3DBlob> error = nullptr;
		HRESULT hr = D3D12SerializeVersionedRootSignature(&versionedDesc, &serializedRootSignature, &error);
		if (FAILED(hr))
		{
			if (error)
			{
				OutputDebugStringA("Root Signature Serialization Error: ");
				OutputDebugStringA((const char*)error->GetBufferPointer());
				OutputDebugStringA("\n");
			}
			throw std::runtime_error("Failed to serialize root signature");
		}

		Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
		ThrowIfFailed(device_->CreateRootSignature(0,
			serializedRootSignature->GetBufferPointer(),
			serializedRootSignature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));

#if defined(_DEBUG)
		rootSignature->SetName(name.c_str());
#endif

		return rootSignature;
	}
	RootSignatureBuilder& RootSignatureBuilder::Reset()
	{
		parameters_.clear();
		staticSamplers_.clear();
		rangeStorage_.clear();

		return *this;
	}
}
