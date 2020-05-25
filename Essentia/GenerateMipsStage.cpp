#include "pch.h"
#include "GenerateMipsStage.h"
#include "Renderer.h"
#include "SceneResources.h"

using namespace DirectX;

void GenerateMipsStage::Initialize()
{
	auto shaderResourceManager = GContext->ShaderResourceManager;

	for (uint32 i = 0; i < CVoxelGridMips; ++i)
	{
		voxelMipGenCBV[i] = shaderResourceManager->CreateCBV(sizeof(MipGenParams));
	}
}

void GenerateMipsStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto renderer = GContext->RendererInstance;
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto computeContext = renderer->GetComputeContext();
	auto computeCList = computeContext->GetDefaultCommandList();

	auto voxelSize = CVoxelSize;

	for (uint32 i = 0; i < GSceneResources.VoxelRadiance.MipCount; ++i)
	{
		voxelSize /= 2;
		MipGenParams params = { XMUINT3(voxelSize, voxelSize, voxelSize), (float)i }; // output resolution
		shaderResourceManager->CopyToCB(frameIndex, { &params, sizeof(params) }, voxelMipGenCBV[i]);
	}

	//TransitionDesc transitions[] = {
	//	{ GSceneResources.VoxelGridResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
	//};
	//renderer->TransitionBarrier(computeCList, transitions, _countof(transitions));

	static constexpr uint32 CMipBlockSize = 4;

	uint32 voxelDispatchSize = CVoxelSize;
	renderer->SetComputeShaderResourceView(computeCList, RootSigComputeSRV, GSceneResources.VoxelGridSRV);
	for (uint32 i = 0; i < CVoxelGridMips - 1; ++i)
	{
		voxelDispatchSize /= 2;
		uint32 dispatchThreadCount = std::max(1u, (voxelDispatchSize + CMipBlockSize - 1) / CMipBlockSize);

		renderer->SetComputeConstantBufferView(computeCList, RootSigComputeCB, voxelMipGenCBV[i]);
		renderer->SetComputeShaderResourceView(computeCList, RootSigComputeUAV, GSceneResources.VoxelRadiance.VoxelGridUAVMips[i + 1]);
		computeCList->Dispatch(dispatchThreadCount, dispatchThreadCount, dispatchThreadCount);
		voxelSize /= 2;
	}

	//TransitionDesc endTransitions[] = {
	//	{ GSceneResources.VoxelGridResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE }
	//};

	//renderer->TransitionBarrier(computeCList, transitions, _countof(endTransitions));
}
