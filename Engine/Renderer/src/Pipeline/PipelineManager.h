#pragma once

#include "Exception.h"
#include "GraphicsPSOBuilder.h"

#include <unordered_map>
#include <functional>
#include <memory>
#include <string>
#include <cassert>

#include <windows.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

namespace renderer::pipeline
{
    class PipelineManager
    {
    private:
        ID3D12Device8* device_;

        std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D12PipelineState>> psoMap_;

    public:
        PipelineManager(ID3D12Device8* device);

    public:
        void CreatePSO(const std::string& name, std::function<void(renderer::pipeline::GraphicsPSOBuilder&)> configFunc);
        ID3D12PipelineState* GetPSO(const std::string& name);

    };
}
