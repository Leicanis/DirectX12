#include "Foundation.h"
#include "Exception.h"

#include <stdexcept>
#include <windows.h>
#if defined(_DEBUG)
#include <dxgidebug.h>
#endif
using namespace Microsoft::WRL;

// Constructor
namespace renderer::core
{
	Foundation::Foundation()
	{
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
		{
			debugController->EnableDebugLayer();
		}
#endif

		CreateFactory();
		CreateDevice();

#if defined(_DEBUG)
		if (device_)
		{
			ComPtr<ID3D12InfoQueue> infoQueue;
			if (SUCCEEDED(device_.As(&infoQueue)))
			{
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
				infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
			}
		}
#endif
	}
}

namespace renderer::core
{
	void Foundation::CreateFactory()
	{
		if (factory_ != nullptr)
		{
			return;
		}

		UINT createFactoryFlags = 0U;
#if defined(_DEBUG)
		createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

		ThrowIfFailed(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(factory_.GetAddressOf())));
	}
	void Foundation::CreateDevice()
	{
		ComPtr<IDXGIAdapter4> adapter = nullptr;
		for (UINT i = 0;
			factory_->EnumAdapterByGpuPreference(
				i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(adapter.GetAddressOf())) != DXGI_ERROR_NOT_FOUND;
			++i)
		{
			HRESULT hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);
			if (SUCCEEDED(hr))
			{
				break;
			}

			adapter.Reset();
		}
		if (adapter == nullptr)
		{
			throw std::runtime_error("No D3D12.1 compatible GPU found!");
		}

		ThrowIfFailed(D3D12CreateDevice(
			adapter.Get(), D3D_FEATURE_LEVEL_12_1, IID_PPV_ARGS(device_.GetAddressOf())));
	}
}
