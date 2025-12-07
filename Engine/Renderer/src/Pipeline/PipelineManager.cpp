#include "PipelineManager.h"

// Constructor
namespace renderer::pipeline
{
    PipelineManager::PipelineManager(ID3D12Device8* device) : device_(device)
    {
        assert(device_ != nullptr && "PipelineManager requires a valid D3D12 device pointer.");
    }
}
namespace renderer::pipeline
{
    void PipelineManager::CreatePSO(const std::string& name, std::function<void(renderer::pipeline::GraphicsPSOBuilder&)> configFunc)
    {
        renderer::pipeline::GraphicsPSOBuilder builder(device_);
        configFunc(builder);
        psoMap_[name] = builder.Build();
    }
    ID3D12PipelineState* PipelineManager::GetPSO(const std::string& name)
    {
        return psoMap_[name].Get();
    }
}
