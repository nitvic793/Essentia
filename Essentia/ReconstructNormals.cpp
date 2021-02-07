#include "ReconstructNormals.h"
#include "RenderTargetManager.h"
#include "Renderer.h"
#include "SceneResources.h"

void ReconstructNormals::Initialize()
{
	GRenderStageManager.RegisterStage(this);
	auto renderer = GContext->RendererInstance;
	auto screenSize = renderer->GetScreenSize();
	normalTarget = CreateSceneRenderTarget(GContext, screenSize.Width, screenSize.Height, DXGI_FORMAT_R11G11B10_FLOAT);
	GSceneResources.SceneNormals = normalTarget;
}

void ReconstructNormals::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto renderer = GContext->RendererInstance;
	auto rtManager = GContext->RenderTargetManager;
	auto commandList = renderer->GetDefaultCommandList();

	renderer->TransitionBarrier(commandList, normalTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(normalTarget.RenderTarget), ColorValues::ClearColor, 0, nullptr);
	renderer->SetRenderTargets(&normalTarget.RenderTarget, 1, nullptr);
	renderer->SetPipelineState(commandList, GPipelineStates.ReconstructNormalsPSO);
	renderer->SetConstantBufferView(commandList, RootSigCBAll1, GSceneResources.FrameDataCBV);
	renderer->SetShaderResourceView(commandList, RootSigSRVPixel1, GSceneResources.DepthPrePass.Texture);

	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, normalTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

void ReconstructNormals::CleanUp()
{
}
