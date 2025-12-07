#pragma once

#include <vector>
#include <string>
#include <list>

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace renderer::pipeline
{
	class InputLayoutBuilder
	{
	private:
		std::vector<D3D12_INPUT_ELEMENT_DESC> elements_;
		std::list<std::string> semanticNames_;

	public:
		InputLayoutBuilder() = default;

	public:
		InputLayoutBuilder& Add(
			std::string semanticName,
			DXGI_FORMAT format,
			UINT offset = D3D12_APPEND_ALIGNED_ELEMENT,
			UINT semanticIndex = 0,
			UINT inputSlot = 0,
			D3D12_INPUT_CLASSIFICATION slotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
			INT instanceStepRate = 0);

		const D3D12_INPUT_LAYOUT_DESC GetDesc() const;

	};
}
