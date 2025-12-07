#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace renderer::core
{
	class Foundation
	{
	private:
		Microsoft::WRL::ComPtr<IDXGIFactory6> factory_;
		Microsoft::WRL::ComPtr<ID3D12Device8> device_;

	private:
		void CreateFactory();
		void CreateDevice();

	public:
		Foundation();

		IDXGIFactory6* GetFactory() const { return factory_.Get(); }
		ID3D12Device8* GetDevice() const { return device_.Get(); }

	};
}
