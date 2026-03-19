#include "CommandContext.h"
#include "CommandQueueManager.h"
#include "../Core/RendererType.h"

#include <cassert>


namespace renderer::command
{
	void LocalThreadCommandPool::RequestContextFromManagerInternal(renderer::core::CommandType type)
	{
		switch (type)
		{
		case renderer::core::CommandType::Graphics:
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> tempAllocator =
				manager_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT);
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> tempList =
				manager_->CreateCommandList(tempAllocator.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);

			std::unique_ptr<GraphicsCommandContext> tempContext =
				std::make_unique<GraphicsCommandContext>(renderer::command::detail::CommandContextBase{ tempAllocator, tempList, graphicsContextCount_ });

			tempContext->Finish();
			availableGraphicsContextIndices_.push(graphicsContextCount_);
			graphicsContexts_[graphicsContextCount_++] = std::move(tempContext);
			break;
		}
		case renderer::core::CommandType::Compute:
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> tempAllocator =
				manager_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE);
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> tempList =
				manager_->CreateCommandList(tempAllocator.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);

			std::unique_ptr<ComputeCommandContext> tempContext =
				std::make_unique<ComputeCommandContext>(renderer::command::detail::CommandContextBase{ tempAllocator, tempList, computeContextCount_ });

			tempContext->Finish();
			availableComputeContextIndices_.push(computeContextCount_);
			computeContexts_[computeContextCount_++] = std::move(tempContext);
			break;
		}
		case renderer::core::CommandType::Copy:
		{
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> tempAllocator =
				manager_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COPY);
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> tempList =
				manager_->CreateCommandList(tempAllocator.Get(), D3D12_COMMAND_LIST_TYPE_COPY);

			std::unique_ptr<CopyCommandContext> tempContext =
				std::make_unique<CopyCommandContext>(renderer::command::detail::CommandContextBase{ tempAllocator , tempList, copyContextCount_ });

			tempContext->Finish();
			availableCopyContextIndices_.push(copyContextCount_);
			copyContexts_[copyContextCount_++] = std::move(tempContext);
			break;
		}
		}
	}
}
// Constructor
namespace renderer::command
{
	LocalThreadCommandPool::LocalThreadCommandPool(
		renderer::command::CommandQueueManager* manager,
		size_t graphicsContextCount = 1, size_t computeContextCount = 0, size_t copyContextCount = 0) : manager_(manager)
	{
		assert(manager_ != nullptr && "LocalThreadCommandPool needs a available CommandQueueManager pointer.");

		for (int i = 0; i < graphicsContextCount; i++)
		{
			RequestContextFromManagerInternal(renderer::core::CommandType::Graphics);
		}
		for (int i = 0; i < computeContextCount; i++)
		{
			RequestContextFromManagerInternal(renderer::core::CommandType::Compute);
		}
		for (int i = 0; i < copyContextCount; i++)
		{
			RequestContextFromManagerInternal(renderer::core::CommandType::Copy);
		}
	}
}
namespace renderer::command
{
	GraphicsCommandContext* LocalThreadCommandPool::RequestGraphicsContext()
	{
		if (!pendingGraphicsContextIndices_.empty())
		{
			uint64_t fenceValue = manager_->GetFenceValue(renderer::core::CommandType::Graphics);
			while (!pendingGraphicsContextIndices_.empty() && pendingGraphicsContextIndices_.front() <= fenceValue)
			{
				availableGraphicsContextIndices_.push(pendingGraphicsContextIndices_.front());
				pendingGraphicsContextIndices_.pop();
			}
		}

		if (!availableGraphicsContextIndices_.empty())
		{
			size_t availableContextIndex = availableGraphicsContextIndices_.front();
			availableGraphicsContextIndices_.pop();

			graphicsContexts_[availableContextIndex]->Reset();
			return graphicsContexts_[availableContextIndex].get();
		}

		if (graphicsContextCount_ < maxContextCount)
		{
			RequestContextFromManagerInternal(renderer::core::CommandType::Graphics);
			return RequestGraphicsContext();
		}

		if (pendingGraphicsContextIndices_.empty())
		{
			assert(false && "RequestGraphicsContext Deadlock! All contexts are active on CPU.");
			return nullptr;
		}

		auto& oldestCommandContext = graphicsContexts_[pendingGraphicsContextIndices_.front()];
		manager_->WaitForFence(renderer::core::CommandType::Graphics, oldestCommandContext->GetSubmitFenceValue());

		return RequestGraphicsContext();
	}
	ComputeCommandContext* LocalThreadCommandPool::RequestComputeContext()
	{
		if (pendingComputeContextIndices_.size() != 0)
		{
			uint64_t fenceValue = manager_->GetFenceValue(renderer::core::CommandType::Compute);
			while (pendingComputeContextIndices_.front() <= fenceValue)
			{
				availableComputeContextIndices_.push(pendingComputeContextIndices_.front());
				pendingComputeContextIndices_.pop();
			}
		}

		if (!availableComputeContextIndices_.empty())
		{
			size_t availableContextIndex = availableComputeContextIndices_.front();
			availableComputeContextIndices_.pop();

			computeContexts_[availableContextIndex]->Reset();
			return computeContexts_[availableContextIndex].get();
		}

		if (computeContextCount_ < maxContextCount)
		{
			RequestContextFromManagerInternal(renderer::core::CommandType::Graphics);
			return RequestComputeContext();
		}

		if (pendingComputeContextIndices_.empty())
		{
			assert(false && "RequestComputeContext Deadlock! All contexts are active on CPU.");
			return nullptr;
		}

		auto& oldestCommandContext = computeContexts_[pendingComputeContextIndices_.front()];
		manager_->WaitForFence(renderer::core::CommandType::Compute, oldestCommandContext->GetSubmitFenceValue());

		return RequestComputeContext();
	}
	CopyCommandContext* LocalThreadCommandPool::RequestCopyContext()
	{
		if (pendingCopyContextIndices_.size() != 0)
		{
			uint64_t fenceValue = manager_->GetFenceValue(renderer::core::CommandType::Copy);
			while (pendingCopyContextIndices_.front() <= fenceValue)
			{
				availableCopyContextIndices_.push(pendingCopyContextIndices_.front());
				pendingCopyContextIndices_.pop();
			}
		}

		if (!availableCopyContextIndices_.empty())
		{
			size_t availableContextIndex = availableCopyContextIndices_.front();
			availableCopyContextIndices_.pop();

			copyContexts_[availableContextIndex]->Reset();
			return copyContexts_[availableContextIndex].get();
		}

		if (copyContextCount_ < maxContextCount)
		{
			RequestContextFromManagerInternal(renderer::core::CommandType::Copy);
			return RequestCopyContext();
		}

		if (pendingCopyContextIndices_.empty())
		{
			assert(false && "RequestComputeContext Deadlock! All contexts are active on CPU.");
			return nullptr;
		}

		auto& oldestCommandContext = copyContexts_[pendingCopyContextIndices_.front()];
		manager_->WaitForFence(renderer::core::CommandType::Copy, oldestCommandContext->GetSubmitFenceValue());

		return RequestCopyContext();
	}

	void LocalThreadCommandPool::SubmitAndSignalGraphicsContext(GraphicsCommandContext* context)
	{
		assert(context->IsFinish() && "GraphicsContext hasn't finish.");

		uint64_t fenceValue = manager_->SubmitCommandListAndSignal(renderer::core::CommandType::Graphics, context->GetCommandListInternal());
		context->SetSubmitFenceValue(fenceValue);

		pendingGraphicsContextIndices_.push(context->GetPoolIndex());
	}
	void LocalThreadCommandPool::SubmitAndSignalComputeContext(ComputeCommandContext* context)
	{
		assert(context->IsFinish() && "ComputeContext hasn't finish.");

		uint64_t fenceValue = manager_->SubmitCommandListAndSignal(renderer::core::CommandType::Compute, context->GetCommandListInternal());
		context->SetSubmitFenceValue(fenceValue);

		pendingComputeContextIndices_.push(context->GetPoolIndex());
	}
	void LocalThreadCommandPool::SubmitAndSignalCopyContext(CopyCommandContext* context)
	{
		assert(context->IsFinish() && "CopyContext hasn't finish.");

		uint64_t fenceValue = manager_->SubmitCommandListAndSignal(renderer::core::CommandType::Copy, context->GetCommandListInternal());
		context->SetSubmitFenceValue(fenceValue);

		pendingCopyContextIndices_.push(context->GetPoolIndex());
	}

	void LocalThreadCommandPool::WaitForGPU(renderer::core::CommandType type)
	{
		manager_->WaitForGPU(type);
	}
}
