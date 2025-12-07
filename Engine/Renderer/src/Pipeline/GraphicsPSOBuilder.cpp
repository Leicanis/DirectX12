#include "GraphicsPSOBuilder.h"
#include "Exception.h"

// Constructor
namespace renderer::pipeline
{
	GraphicsPSOBuilder::GraphicsPSOBuilder(ID3D12Device8* device) : device_(device)
	{
		ZeroMemory(&desc_, sizeof(desc_));

		D3D12_RASTERIZER_DESC rasterizerDesc{};
		rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
		rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;

		D3D12_BLEND_DESC blendDesc{};
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

		desc_.RasterizerState = rasterizerDesc;
		desc_.BlendState = blendDesc;
		desc_.DepthStencilState = {};
		desc_.SampleMask = UINT_MAX;
		desc_.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc_.NumRenderTargets = 1U;
		desc_.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc_.SampleDesc.Count = 1U;
	}
}
namespace renderer::pipeline
{
	GraphicsPSOBuilder& GraphicsPSOBuilder::SetRootSignature(ID3D12RootSignature* rootSignature)
	{
		desc_.pRootSignature = rootSignature;
		return *this;
	}
	GraphicsPSOBuilder& GraphicsPSOBuilder::SetShader(D3D12_SHADER_BYTECODE vs, D3D12_SHADER_BYTECODE ps)
	{
		desc_.VS = vs;
		desc_.PS = ps;
		return *this;
	}
	GraphicsPSOBuilder& GraphicsPSOBuilder::SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layout)
	{
		desc_.InputLayout = layout;
		return *this;
	}
	GraphicsPSOBuilder& GraphicsPSOBuilder::SetRTVFormat(DXGI_FORMAT format, DXGI_FORMAT dsvFormat)
	{
		desc_.NumRenderTargets = 1;
		desc_.RTVFormats[0] = format;
		desc_.DSVFormat = dsvFormat;
		if (dsvFormat != DXGI_FORMAT_UNKNOWN) {
			desc_.DepthStencilState.DepthEnable = TRUE;
		}
		return *this;
	}
	GraphicsPSOBuilder& GraphicsPSOBuilder::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TYPE)
	{
		desc_.PrimitiveTopologyType = TYPE;
		return *this;
	}
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GraphicsPSOBuilder::Build()
	{
		Microsoft::WRL::ComPtr<ID3D12PipelineState> pso;
		ThrowIfFailed(device_->CreateGraphicsPipelineState(&desc_, IID_PPV_ARGS(&pso)));
		return pso;
	}
}
