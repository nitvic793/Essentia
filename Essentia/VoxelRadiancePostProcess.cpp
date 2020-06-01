#include "pch.h"
#include "VoxelRadiancePostProcess.h"
#include "Renderer.h"
#include "SceneResources.h"
#include "PipelineStates.h"

void VoxelRadiancePostProcess::Initialize()
{
}

void VoxelRadiancePostProcess::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto renderer = GContext->RendererInstance;
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto computeContext = renderer->GetComputeContext();
	auto computeCList = computeContext->GetDefaultCommandList();

	auto voxelSize = CVoxelSize;
	static constexpr uint32 CMipBlockSize = 4;

	TransitionDesc transitions[] = {
		{ GSceneResources.VoxelGridResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
	};

	//renderer->TransitionBarrier(computeCList, transitions, _countof(transitions));

	uint32 voxelDispatchSize = CVoxelSize;

	computeCList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.VoxelCopyPSO));
	renderer->SetComputeShaderResourceView(computeCList, RootSigComputeSRV, GSceneResources.VoxelGridSRV); 
	renderer->SetComputeShaderResourceView(computeCList, RootSigComputeUAV, GSceneResources.VoxelRadiance.VoxelGridUAV);
	computeCList->Dispatch(voxelDispatchSize, voxelDispatchSize, voxelDispatchSize);


	TransitionDesc endTransitions[] = {
		{ GSceneResources.VoxelGridResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE }
	};

	//renderer->TransitionBarrier(computeCList, endTransitions, _countof(endTransitions));
}
