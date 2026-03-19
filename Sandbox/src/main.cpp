#include "Exception.h"
#include "../../Engine/Platform/src/Window.h"

#include "../../Engine/Renderer/src/Core/Foundation.h"
#include "../../Engine/Renderer/src/Command/CommandQueueManager.h"
#include "../../Engine/Renderer/src/Command/CommandContext.h"
#include "../../Engine/Renderer/src/Resources/ResourceManager.h"
#include "../../Engine/Renderer/src/Resources/GPUBuffer.h"
#include "../../Engine/Renderer/src/Descriptors/DescriptorHeapManager.h"
#include "../../Engine/Renderer/src/Pipeline/ShaderManager.h"
#include "../../Engine/Renderer/src/Pipeline/RootSignatureBuilder.h"
#include "../../Engine/Renderer/src/Pipeline/GraphicsPSOBuilder.h"
#include "../../Engine/Renderer/src/Pipeline/InputLayoutBuilder.h"
#include "../../Engine/Renderer/src/SwapChain/SwapChainManager.h"

#include "../../Engine/Renderer/src/Utils/D3D12Helpers.h"
#include "../../Engine/Renderer/src/Core/RendererType.h"

#include <memory>
#include <iostream>
#include <algorithm>

#include <Windows.h>
#include <DirectXColors.h>
#include <DirectXMath.h>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#if defined(_DEBUG)
#include <dxgidebug.h>
#endif

#define WIDTH 640
#define HEIGHT 480

const float scale = 2.0f;
struct VERTEX
{
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT4 color;
};
VERTEX cubeVertices[] = {
	//前面
	{{-0.5f * scale,  0.5f * scale, -0.5f * scale}, DirectX::XMFLOAT4(DirectX::Colors::Gray) },		// 左上
	{{ 0.5f * scale,  0.5f * scale, -0.5f * scale}, DirectX::XMFLOAT4(DirectX::Colors::Aquamarine) },	// 右上
	{{ 0.5f * scale, -0.5f * scale, -0.5f * scale}, DirectX::XMFLOAT4(DirectX::Colors::Gray) },		// 右下
	{{-0.5f * scale, -0.5f * scale, -0.5f * scale}, DirectX::XMFLOAT4(DirectX::Colors::Aquamarine) },	// 左下
	//後面
	{{-0.5f * scale,  0.5f * scale,  0.5f * scale}, DirectX::XMFLOAT4(DirectX::Colors::Aquamarine) },	// 左上
	{{ 0.5f * scale,  0.5f * scale,  0.5f * scale}, DirectX::XMFLOAT4(DirectX::Colors::Gray) },		// 右上
	{{ 0.5f * scale, -0.5f * scale,  0.5f * scale}, DirectX::XMFLOAT4(DirectX::Colors::Aquamarine) },	// 右下
	{{-0.5f * scale, -0.5f * scale,  0.5f * scale}, DirectX::XMFLOAT4(DirectX::Colors::Gray) },		// 左下

	//--------框--------
	//前面
	{{-0.5f * scale,  0.5f * scale, -0.5f * scale}, {0.0f, 0.0f, 0.0f, 1.0f}},  // 左上
	{{ 0.5f * scale,  0.5f * scale, -0.5f * scale}, {0.0f, 0.0f, 0.0f, 1.0f}},  // 右上
	{{ 0.5f * scale, -0.5f * scale, -0.5f * scale}, {0.0f, 0.0f, 0.0f, 1.0f}},  // 右下
	{{-0.5f * scale, -0.5f * scale, -0.5f * scale}, {0.0f, 0.0f, 0.0f, 1.0f}},  // 左下
	//後面
	{{-0.5f * scale,  0.5f * scale,  0.5f * scale}, {0.0f, 0.0f, 0.0f, 1.0f}},  // 左上
	{{ 0.5f * scale,  0.5f * scale,  0.5f * scale}, {0.0f, 0.0f, 0.0f, 1.0f}},  // 右上
	{{ 0.5f * scale, -0.5f * scale,  0.5f * scale}, {0.0f, 0.0f, 0.0f, 1.0f}},  // 右下
	{{-0.5f * scale, -0.5f * scale,  0.5f * scale}, {0.0f, 0.0f, 0.0f, 1.0f}},  // 左下
};
UINT16 cubeIndices[] = {

	0, 1, 2, 0, 2, 3,// 前面
	4, 6, 5, 4, 7, 6,// 後面
	4, 0, 3, 4, 3, 7,// 左面
	1, 5, 6, 1, 6, 2,// 右面
	4, 5, 1, 4, 1, 0,// 上面
	3, 2, 6, 3, 6, 7,// 下面

	//--------框--------
	0 + 8, 1 + 8, 2 + 8, 0 + 8, 2 + 8, 3 + 8,	// 前面
	4 + 8, 6 + 8, 5 + 8, 4 + 8, 7 + 8, 6 + 8,	// 後面
	4 + 8, 0 + 8, 3 + 8, 4 + 8, 3 + 8, 7 + 8,	// 左面
	1 + 8, 5 + 8, 6 + 8, 1 + 8, 6 + 8, 2 + 8,	// 右面
	4 + 8, 5 + 8, 1 + 8, 4 + 8, 1 + 8, 0 + 8,	// 上面
	3 + 8, 2 + 8, 6 + 8, 3 + 8, 6 + 8, 7 + 8,	// 下面
	1 + 8, 2 + 8, 4 + 8, 7 + 8, 0 + 8, 3 + 8, 5 + 8, 6 + 8 //???
};

DirectX::XMFLOAT4X4 worldViewProj = renderer::utils::MathHelper::Identity4x4();
float theta = 1.5f * DirectX::XM_PI;
float phi = DirectX::XM_PIDIV4;
float radius = 5.0f;

void SetWorldProj(DirectX::XMFLOAT4X4& objConstants)
{
	DirectX::XMFLOAT4X4 float4x4_world{};
	DirectX::XMFLOAT4X4 float4x4_view{};
	DirectX::XMFLOAT4X4 float4x4_proj{};
	DirectX::XMStoreFloat4x4(&float4x4_world, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&float4x4_view, DirectX::XMMatrixIdentity());
	DirectX::XMStoreFloat4x4(&float4x4_proj, DirectX::XMMatrixIdentity());
	DirectX::XMMATRIX P = DirectX::XMMatrixPerspectiveFovLH(
		0.25f * DirectX::XM_PI, static_cast<float>(WIDTH) / HEIGHT, 1.0f, 1000.0f);
	XMStoreFloat4x4(&float4x4_proj, P);

	phi = std::clamp(phi, 0.1f, DirectX::XM_PI - 0.1f);
	float x = radius * sinf(phi) * cosf(theta);
	float z = radius * sinf(phi) * sinf(theta);
	float y = radius * cosf(phi);
	// Build the view matrix.
	DirectX::XMVECTOR pos = DirectX::XMVectorSet(x, y, z, 1.0f);
	DirectX::XMVECTOR target = DirectX::XMVectorZero();
	DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	DirectX::XMMATRIX view = DirectX::XMMatrixLookAtLH(pos, target, up);
	DirectX::XMStoreFloat4x4(&float4x4_view, view);
	DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&float4x4_world);
	DirectX::XMMATRIX proj = DirectX::XMLoadFloat4x4(&float4x4_proj);
	DirectX::XMMATRIX worldViewProj = world * view * proj;
	// Update the constant buffer with the latest worldViewProj matrix.
	XMStoreFloat4x4(&objConstants, XMMatrixTranspose(worldViewProj));
}

int WINAPI
wWinMain(
	_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance,
	_In_ PWSTR nCmdLine, _In_ int nCmdShow)
{
#if defined(DEBUG) || defined(_DEBUG)
	ComPtr<ID3D12Debug> debugController;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(debugController.GetAddressOf())));
	debugController->EnableDebugLayer();
#endif

	platform::Window window;
	window.RegisterWindowClass(hInstance, CS_HREDRAW | CS_VREDRAW, L"Window");
	window.Create(hInstance, L"DirectX test", WIDTH, HEIGHT, WS_VISIBLE | WS_OVERLAPPEDWINDOW, L"Window");
	window.RegisterMessageHandler(WM_DESTROY, [](WPARAM wParam, LPARAM lParam) { ::PostQuitMessage(0); });
	window.ShowWindow();

	{
		std::unique_ptr<renderer::core::Foundation> foundation{};
		try
		{
			foundation = std::make_unique<renderer::core::Foundation>();
		}
		catch (std::runtime_error& err)
		{
			std::cerr << err.what() << std::endl;
			return -1;
		}

		std::unique_ptr<renderer::command::CommandQueueManager> commandQueueManager =
			std::make_unique<renderer::command::CommandQueueManager>(foundation->GetDevice());
		std::unique_ptr<renderer::core::ResourceManager> resourceManager =
			std::make_unique<renderer::core::ResourceManager>(foundation->GetDevice(), commandQueueManager.get());
		std::unique_ptr<renderer::descriptors::DescriptorHeapManager> descriptorHeapManager =
			std::make_unique<renderer::descriptors::DescriptorHeapManager>(foundation->GetDevice());
		std::unique_ptr<renderer::pipeline::ShaderManager> shaderManager = std::make_unique<renderer::pipeline::ShaderManager>();

		std::unique_ptr<renderer::display::SwapChainManager> swapChainManager =
			std::make_unique<renderer::display::SwapChainManager>(
				foundation->GetDevice(), foundation->GetFactory(), commandQueueManager->GetCommandQueue(renderer::core::CommandType::Graphics),
				commandQueueManager.get(), descriptorHeapManager.get(), window.GetHandle(), WIDTH, HEIGHT);

		std::unique_ptr<renderer::command::LocalThreadCommandPool> threadCommandPool =
			std::make_unique<renderer::command::LocalThreadCommandPool>(commandQueueManager.get(), 1, 0, 0);
		renderer::command::GraphicsCommandContext* context = threadCommandPool->RequestGraphicsContext();

		//=-------- ConstantBuffer --------=//
		SetWorldProj(worldViewProj);
		renderer::core::DynamicAllocation allocation =
			resourceManager->CreateDynamicBuffer(renderer::utils::CalculateConstantBufferByteSize(sizeof(worldViewProj)));
		if (allocation.IsValid())
		{
			memcpy(allocation.cpuAddress, &worldViewProj, sizeof(worldViewProj));
		}

		//=-------- GPU Data --------=//
		renderer::core::ResourceHandle vertexBufferHandle = resourceManager->CreateStaticBuffer(*context, &cubeVertices, sizeof(cubeVertices));
		renderer::core::ResourceHandle indexBufferHandle = resourceManager->CreateStaticBuffer(*context, &cubeIndices, sizeof(cubeIndices));
		assert(vertexBufferHandle.GetIndex() != indexBufferHandle.GetIndex() && "GetIndex same");
		assert(vertexBufferHandle.GetVersion() == indexBufferHandle.GetVersion() && "GetVersion different");

		assert(resourceManager->GetStaticResource(vertexBufferHandle) != nullptr && "StaticResource same");
		assert(resourceManager->GetStaticResource(indexBufferHandle) != nullptr && "StaticResource same");

		renderer::resources::GPUBuffer* vertexBufferResource = static_cast<renderer::resources::GPUBuffer*>(resourceManager->GetStaticResource(vertexBufferHandle));
		renderer::resources::GPUBuffer* indexBufferResource = static_cast<renderer::resources::GPUBuffer*>(resourceManager->GetStaticResource(indexBufferHandle));

		context->TransitionResource(vertexBufferResource->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		context->TransitionResource(indexBufferResource->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_INDEX_BUFFER);
		vertexBufferResource->SetState(D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
		indexBufferResource->SetState(D3D12_RESOURCE_STATE_INDEX_BUFFER);

		context->Finish();
		threadCommandPool->SubmitAndSignalGraphicsContext(context);
		threadCommandPool->WaitForGPU(renderer::core::CommandType::Graphics);
		context = threadCommandPool->RequestGraphicsContext();

		D3D12_VERTEX_BUFFER_VIEW vertexBufferView = vertexBufferResource->GetVertexBufferView(sizeof(VERTEX));
		D3D12_INDEX_BUFFER_VIEW indexBufferView = indexBufferResource->GetIndexBufferView(DXGI_FORMAT_R16_UINT);

		//=-------- RootSignature --------=//
		renderer::pipeline::RootSignatureBuilder rootSignatureBuilder(foundation->GetDevice());
		D3D12_DESCRIPTOR_RANGE1 cbvTable{};
		cbvTable.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvTable.NumDescriptors = 1;
		cbvTable.RegisterSpace = 0;
		cbvTable.BaseShaderRegister = 0;
		cbvTable.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		cbvTable.Flags = D3D12_DESCRIPTOR_RANGE_FLAG_NONE;
		ComPtr<ID3D12RootSignature> rootSignature = rootSignatureBuilder.
			AddDescriptorTable({ cbvTable }, D3D12_SHADER_VISIBILITY_ALL).
			Build(D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT, L"RootSignature1");

		//=-------- Compile Shader --------=//
		IDxcBlob* vsBlob = shaderManager->GetShader(L"Shaders\\color.hlsl", L"VSMain", L"vs_6_0");
		IDxcBlob* psBlob = shaderManager->GetShader(L"Shaders\\color.hlsl", L"PSMain", L"ps_6_0");

		//=-------- Pipeline State --------=//
		renderer::pipeline::InputLayoutBuilder inputLayoutBuilder;
		D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = inputLayoutBuilder.
			Add("POSITION", DXGI_FORMAT_R32G32B32_FLOAT, 0U).
			Add("COLOR", DXGI_FORMAT_R32G32B32A32_FLOAT, 12U).
			GetDesc();

		renderer::pipeline::GraphicsPSOBuilder graphicsPSOBuilder(foundation->GetDevice());

		ComPtr<ID3D12PipelineState> trianglePipelineState = graphicsPSOBuilder.
			SetInputLayout(inputLayoutDesc).
			SetShader({ reinterpret_cast<BYTE*>(vsBlob->GetBufferPointer()), vsBlob->GetBufferSize() },
				{ reinterpret_cast<BYTE*>(psBlob->GetBufferPointer()), psBlob->GetBufferSize() }).
			SetRootSignature(rootSignature.Get()).
			SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN).
			SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE).
			Build();

		ComPtr<ID3D12PipelineState> linePipelineState = graphicsPSOBuilder.
			SetInputLayout(inputLayoutDesc).
			SetShader({ reinterpret_cast<BYTE*>(vsBlob->GetBufferPointer()), vsBlob->GetBufferSize() },
				{ reinterpret_cast<BYTE*>(psBlob->GetBufferPointer()), psBlob->GetBufferSize() }).
			SetRootSignature(rootSignature.Get()).
			SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_UNKNOWN).
			SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE).
			Build();

		//=-------- CBV setting --------=//
		renderer::core::DescriptorHandle cbvDescriptorHandle = descriptorHeapManager->CreateCBV(allocation.gpuAddress, sizeof(worldViewProj));

		//=-------- commandList setting --------=//
		context->IASetVertexBuffers(0, 1, { &vertexBufferView });
		context->IASetIndexBuffer(&indexBufferView);
		context->SetGraphicsRootSignature(rootSignature.Get());
		ID3D12DescriptorHeap* descriptorHeaps[] = { descriptorHeapManager->GetCSUHeap() };
		context->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
		context->SetGraphicsRootDescriptorTable(0, cbvDescriptorHandle.gpuHandle);
		D3D12_VIEWPORT viewport = {
		0.0f, 0.0f,
		static_cast<float>(WIDTH),
		static_cast<float>(HEIGHT),
		D3D12_MIN_DEPTH, D3D12_MAX_DEPTH };
		D3D12_RECT scissorRect = {
			0, 0,
			static_cast<LONG>(WIDTH),
			static_cast<LONG>(HEIGHT) };
		context->RSSetViewports(1, &viewport);
		context->RSSetScissorRects(1, &scissorRect);

		//=-------- SwapChain --------=//
		float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
		swapChainManager->Clear(*context, clearColor);

		context->SetPipelineState(trianglePipelineState.Get());
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->DrawIndexedInstanced(36, 1, 0, 0, 0);
		context->SetPipelineState(linePipelineState.Get());
		context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_LINELIST);
		context->DrawIndexedInstanced(44, 1, 36, 0, 0);

		swapChainManager->PreparePresent(*context);
		context->Finish();
		threadCommandPool->SubmitAndSignalGraphicsContext(context);

		swapChainManager->Present();

		threadCommandPool->WaitForGPU(renderer::core::CommandType::Graphics);
	}

#if defined(_DEBUG)
	ComPtr<IDXGIDebug1> dxgiDebug;
	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&dxgiDebug))))
	{
		dxgiDebug->ReportLiveObjects(
			DXGI_DEBUG_ALL,
			DXGI_DEBUG_RLO_FLAGS(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL)
		);
	}
#endif

	return window.RunMessageLoop();
}
