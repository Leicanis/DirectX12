#pragma once

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace renderer::pipeline
{
	class GraphicsPSOBuilder
	{
	private:
		ID3D12Device8* device_;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc_;

	public:
		GraphicsPSOBuilder(ID3D12Device8* device);

	public:
		GraphicsPSOBuilder& SetRootSignature(ID3D12RootSignature* rootSignature);
		GraphicsPSOBuilder& SetShader(D3D12_SHADER_BYTECODE vs, D3D12_SHADER_BYTECODE ps);
		GraphicsPSOBuilder& SetInputLayout(const D3D12_INPUT_LAYOUT_DESC& layout);
		GraphicsPSOBuilder& SetRTVFormat(DXGI_FORMAT format, DXGI_FORMAT dsvFormat = DXGI_FORMAT_UNKNOWN);
		GraphicsPSOBuilder& SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE TYPE);

		// SetBlendState, SetWireframe, SetCullMode...

		Microsoft::WRL::ComPtr<ID3D12PipelineState> Build();

	};
}
