#include "InputLayoutBuilder.h"

namespace renderer::pipeline
{
	InputLayoutBuilder& InputLayoutBuilder::Add(
		std::string semanticName,
		DXGI_FORMAT format,
		UINT offset,
		UINT semanticIndex,
		UINT inputSlot,
		D3D12_INPUT_CLASSIFICATION slotClass,
		INT instanceStepRate)
	{
		semanticNames_.push_back(semanticName);
		const char* semanticNamePtr = semanticNames_.back().c_str();

		D3D12_INPUT_ELEMENT_DESC tempElement{};
		tempElement.SemanticName = semanticNamePtr;
		tempElement.Format = format;
		tempElement.SemanticIndex = semanticIndex;
		tempElement.AlignedByteOffset = offset;
		tempElement.InputSlot = inputSlot;
		tempElement.InputSlotClass = slotClass;
		tempElement.InstanceDataStepRate = instanceStepRate;

		elements_.push_back(tempElement);

		return *this;
	}

	const D3D12_INPUT_LAYOUT_DESC InputLayoutBuilder::GetDesc() const
	{
		return { elements_.data(), (UINT)elements_.size() };
	}
}
