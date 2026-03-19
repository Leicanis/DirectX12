#pragma once

#include "../Core/RendererType.h"

#include <array>
#include <memory>
#include <queue>
#include <cassert>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

// Forward declaration
namespace renderer::command
{
	class CommandQueueManager;
}
namespace renderer::command
{
	class LocalThreadCommandPool;
}

namespace renderer::command
{
	namespace detail
	{
		class CommandContextBase
		{
		private:
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator_ = nullptr;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> list_ = nullptr;
			uint64_t submitFenceValue_ = 0;
			size_t poolIndex_;
			bool isClosed_ = false;

		public:
			CommandContextBase(
				const Microsoft::WRL::ComPtr<ID3D12CommandAllocator>& allocator,
				const Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>& list,
				size_t poolIndex) :
				allocator_(allocator), list_(list), poolIndex_(poolIndex) {
			}

		public:
			void Reset()
			{
				list_->Reset(allocator_.Get(), nullptr);
			}
			void Close()
			{
				list_->Close();
				isClosed_ = true;
			}
			bool IsClosed() { return isClosed_; }

			ID3D12GraphicsCommandList* GetList() const { return list_.Get(); }
			ID3D12CommandAllocator* GetAllocator() const { return allocator_.Get(); }

			void SetSubmitFenceValue(uint64_t fenceValue) { submitFenceValue_ = fenceValue; }
			uint64_t GetSubmitFenceValue() const { return submitFenceValue_; }

			size_t GetPoolIndex() { return poolIndex_; }

		};
	}

	class GraphicsCommandContext
	{
		friend class renderer::command::LocalThreadCommandPool;

	private:
		renderer::command::detail::CommandContextBase baseContext_;

	protected:
		ID3D12GraphicsCommandList* GetCommandListInternal() { return baseContext_.GetList(); }

	public:
		GraphicsCommandContext(renderer::command::detail::CommandContextBase base) : baseContext_(base) {}

		void Reset()
		{
			baseContext_.Reset();
		}
		void Finish()
		{
			baseContext_.Close();
		}
		bool IsFinish() { return baseContext_.IsClosed(); }

		void SetSubmitFenceValue(uint64_t fenceValue) { baseContext_.SetSubmitFenceValue(fenceValue); }
		uint64_t GetSubmitFenceValue() const { return baseContext_.GetSubmitFenceValue(); }

		size_t GetPoolIndex() { return baseContext_.GetPoolIndex(); }

		//=-------- other graphicsCommandList command --------=//
		void CopyBuffer(ID3D12Resource* dest, ID3D12Resource* src, UINT64 size, UINT64 destOffset = 0, UINT64 srcOffset = 0)
		{
			baseContext_.GetList()->CopyBufferRegion(dest, destOffset, src, srcOffset, size);
		}
		void CopyResource(ID3D12Resource* dest, ID3D12Resource* src)
		{
			baseContext_.GetList()->CopyResource(dest, src);
		}
		void CopyTextureRegion(
			const D3D12_TEXTURE_COPY_LOCATION* dest, UINT x, UINT y, UINT z,
			const D3D12_TEXTURE_COPY_LOCATION* src,
			const D3D12_BOX* srcBox)
		{
			baseContext_.GetList()->CopyTextureRegion(dest, x, y, z, src, srcBox);
		}

		void TransitionResource(
			ID3D12Resource* resource, D3D12_RESOURCE_STATES before, D3D12_RESOURCE_STATES after)
		{
			D3D12_RESOURCE_BARRIER barrier;
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = resource;
			barrier.Transition.StateBefore = before;
			barrier.Transition.StateAfter = after;
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

			baseContext_.GetList()->ResourceBarrier(1, &barrier);
		}

		void DrawIndexedInstanced(
			UINT IndexCountPerInstance,
			UINT InstanceCount,
			UINT StartIndexLocation,
			INT BaseVertexLocation,
			UINT StartInstanceLocation)
		{
			baseContext_.GetList()->DrawIndexedInstanced(
				IndexCountPerInstance,
				InstanceCount,
				StartIndexLocation,
				BaseVertexLocation,
				StartInstanceLocation);
		}

		void SetRenderTargets(
			UINT numRTVDescriptors,
			const renderer::core::DescriptorHandle* rtvDescriptorsHandle,
			const renderer::core::DescriptorHandle* dsvDescriptorHandle)
		{
			assert(numRTVDescriptors <= 8 && "SetRenderTargets error: numRTVDescriptors must less than 8.");
			D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[8] = {};
			for (UINT i = 0; i < numRTVDescriptors; ++i)
			{
				rtvHandles[i] = rtvDescriptorsHandle[i].cpuHandle;
			}

			D3D12_CPU_DESCRIPTOR_HANDLE* pDSV = nullptr;
			D3D12_CPU_DESCRIPTOR_HANDLE dsvHandleRaw;
			if (dsvDescriptorHandle)
			{
				dsvHandleRaw = dsvDescriptorHandle->cpuHandle;
				pDSV = &dsvHandleRaw;
			}

			baseContext_.GetList()->OMSetRenderTargets(numRTVDescriptors, rtvHandles, FALSE, pDSV);
		}
		void ClearRenderTarget(renderer::core::DescriptorHandle& rtvDescriptorHandle, const FLOAT colorRGBA[4])
		{
			baseContext_.GetList()->ClearRenderTargetView(rtvDescriptorHandle.cpuHandle, colorRGBA, 0, nullptr);
		}
		void ClearDepthStencil(renderer::core::DescriptorHandle& dsvDescriptorsHandle)
		{
			baseContext_.GetList()->ClearDepthStencilView(dsvDescriptorsHandle.cpuHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);
		}

		void IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY PrimitiveTopology)
		{
			baseContext_.GetList()->IASetPrimitiveTopology(PrimitiveTopology);
		}
		void IASetVertexBuffers(UINT startSlot, UINT numViews, const D3D12_VERTEX_BUFFER_VIEW* pViews)
		{
			baseContext_.GetList()->IASetVertexBuffers(startSlot, numViews, pViews);
		}
		void IASetIndexBuffer(const D3D12_INDEX_BUFFER_VIEW* pViews)
		{
			baseContext_.GetList()->IASetIndexBuffer(pViews);
		}
		void SetGraphicsRootSignature(ID3D12RootSignature* pRootSignature)
		{
			baseContext_.GetList()->SetGraphicsRootSignature(pRootSignature);
		}
		void SetDescriptorHeaps(UINT numDescriptorHeaps, ID3D12DescriptorHeap* const* ppDescriptorHeaps)
		{
			baseContext_.GetList()->SetDescriptorHeaps(numDescriptorHeaps, ppDescriptorHeaps);
		}
		void SetGraphicsRootDescriptorTable(UINT rootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor)
		{
			baseContext_.GetList()->SetGraphicsRootDescriptorTable(rootParameterIndex, baseDescriptor);
		}
		void SetPipelineState(ID3D12PipelineState* pPipelineState)
		{
			baseContext_.GetList()->SetPipelineState(pPipelineState);
		}
		void RSSetViewports(UINT numViewports, const D3D12_VIEWPORT* pViewports)
		{
			baseContext_.GetList()->RSSetViewports(numViewports, pViewports);
		}
		void RSSetScissorRects(UINT numRects, const D3D12_RECT* pRects)
		{
			baseContext_.GetList()->RSSetScissorRects(numRects, pRects);
		}

	};
	class ComputeCommandContext
	{
		friend class renderer::command::LocalThreadCommandPool;

	private:
		renderer::command::detail::CommandContextBase baseContext_;

	protected:
		ID3D12GraphicsCommandList* GetCommandListInternal() { return baseContext_.GetList(); }

	public:
		ComputeCommandContext(renderer::command::detail::CommandContextBase base) : baseContext_(base) {}

		void Reset()
		{
			baseContext_.Reset();
		}
		void Finish()
		{
			baseContext_.Close();
		}
		bool IsFinish() { return baseContext_.IsClosed(); }

		void SetSubmitFenceValue(uint64_t fenceValue) { baseContext_.SetSubmitFenceValue(fenceValue); }
		uint64_t GetSubmitFenceValue() const { return baseContext_.GetSubmitFenceValue(); }

		size_t GetPoolIndex() { return baseContext_.GetPoolIndex(); }

		//=-------- other graphicsCommandList command --------=//
		void CopyBuffer(ID3D12Resource* dest, ID3D12Resource* src, UINT64 size, UINT64 destOffset = 0, UINT64 srcOffset = 0)
		{
			baseContext_.GetList()->CopyBufferRegion(dest, destOffset, src, srcOffset, size);
		}

	};
	class CopyCommandContext
	{
		friend class renderer::command::LocalThreadCommandPool;

	private:
		renderer::command::detail::CommandContextBase baseContext_;

	protected:
		ID3D12GraphicsCommandList* GetCommandListInternal() { return baseContext_.GetList(); }

	public:
		CopyCommandContext(renderer::command::detail::CommandContextBase base) : baseContext_(base) {}

		void Reset()
		{
			baseContext_.Reset();
		}
		void Finish()
		{
			baseContext_.Close();
		}
		bool IsFinish() { return baseContext_.IsClosed(); }

		void SetSubmitFenceValue(uint64_t fenceValue) { baseContext_.SetSubmitFenceValue(fenceValue); }
		uint64_t GetSubmitFenceValue() const { return baseContext_.GetSubmitFenceValue(); }

		size_t GetPoolIndex() { return baseContext_.GetPoolIndex(); }

		//=-------- other graphicsCommandList command --------=//
		void CopyBuffer(ID3D12Resource* dest, ID3D12Resource* src, UINT64 size, UINT64 destOffset = 0, UINT64 srcOffset = 0)
		{
			baseContext_.GetList()->CopyBufferRegion(dest, destOffset, src, srcOffset, size);
		}

	};
}

namespace renderer::command
{
	class LocalThreadCommandPool
	{
	private:
		renderer::command::CommandQueueManager* manager_;
		static const size_t maxContextCount = 16;

		std::array<std::unique_ptr<GraphicsCommandContext>, maxContextCount> graphicsContexts_;
		std::array<std::unique_ptr<ComputeCommandContext>, maxContextCount> computeContexts_;
		std::array<std::unique_ptr<CopyCommandContext>, maxContextCount> copyContexts_;

		size_t graphicsContextCount_ = 0;
		size_t computeContextCount_ = 0;
		size_t copyContextCount_ = 0;

		std::queue<size_t> availableGraphicsContextIndices_;
		std::queue<size_t> availableComputeContextIndices_;
		std::queue<size_t> availableCopyContextIndices_;

		std::queue<size_t> pendingGraphicsContextIndices_;
		std::queue<size_t> pendingComputeContextIndices_;
		std::queue<size_t> pendingCopyContextIndices_;

	private:
		void RequestContextFromManagerInternal(renderer::core::CommandType type);

	public:
		LocalThreadCommandPool(
			renderer::command::CommandQueueManager* manager,
			size_t graphicsContextCount, size_t computeContextCount, size_t copyContextCount);

	public:
		GraphicsCommandContext* RequestGraphicsContext();
		ComputeCommandContext* RequestComputeContext();
		CopyCommandContext* RequestCopyContext();

		void SubmitAndSignalGraphicsContext(GraphicsCommandContext* context);
		void SubmitAndSignalComputeContext(ComputeCommandContext* context);
		void SubmitAndSignalCopyContext(CopyCommandContext* context);

		void WaitForGPU(renderer::core::CommandType type);

	};
}
