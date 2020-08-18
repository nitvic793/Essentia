#include "pch.h"
#include "VolumetricLightStage.h"
#include "RenderTargetManager.h"
#include "PostProcess.h"
#include "Renderer.h"
#include "Entity.h"
#include "PostProcessComponents.h"
#include "Mesh.h"
#include "PipelineStates.h"
#include "SceneResources.h"

struct LightAccumParams
{
	DirectX::XMFLOAT4X4 InvProjection;
	DirectX::XMFLOAT4X4 InvView;
	DirectX::XMFLOAT4X4 World;
	DirectX::XMUINT2	ScreenResolution;
};

struct BilateralBlurParams
{
	DirectX::XMUINT2	FullResolutionSize;
	DirectX::XMUINT2	LowResolutionSize;
};

void VolumetricLightStage::Initialize()
{
	auto renderer = GContext->RendererInstance;
	auto fullSize = renderer->GetScreenSize();
	auto entityManager = GContext->EntityManager;
	auto halfResSize = GPostProcess.GetPostSceneTextures().HalfResSize;
	lightAccumTarget = CreateSceneRenderTarget(GContext, fullSize.Width, fullSize.Height, renderer->GetHDRRenderTargetFormat());
	intermediatelightAccumTarget = CreateSceneRenderTarget(GContext, halfResSize.Width, halfResSize.Height, renderer->GetHDRRenderTargetFormat());
	blurFinalTarget = CreateSceneRenderTarget(GContext, halfResSize.Width, halfResSize.Height, renderer->GetHDRRenderTargetFormat());
	blurIntermediateTarget = CreateSceneRenderTarget(GContext, halfResSize.Width, halfResSize.Height, renderer->GetHDRRenderTargetFormat());
	GSceneResources.LightAccumTarget = lightAccumTarget;
	uint32 count = 0;
	auto entities = entityManager->GetEntities<PostProcessVolumeComponent>(count);
	if (count == 0)
	{
		volumeEntity = GContext->EntityManager->CreateEntity();
		GContext->EntityManager->AddComponent<PostProcessVolumeComponent>(volumeEntity);
		GContext->EntityManager->AddComponent<BaseDrawableComponent>(volumeEntity, BaseDrawableComponent::Create());
		auto transform = GContext->EntityManager->GetTransform(volumeEntity);
		//*transform.Scale = DirectX::XMFLOAT3(100.f, 100.f, 100.f);
	}
	else
	{
		volumeEntity = entities[0];
	}

	lightAccumCBV = GContext->ShaderResourceManager->CreateCBV(sizeof(LightAccumParams));
	bilateralBlurCBV = GContext->ShaderResourceManager->CreateCBV(sizeof(BilateralBlurParams));

	blurHorizontalCBV = GContext->ShaderResourceManager->CreateCBV(sizeof(BlurParams));
	blurVerticalCBV = GContext->ShaderResourceManager->CreateCBV(sizeof(BlurParams));

	MeshView meshView;
	cubeMesh = GContext->MeshManager->CreateMesh("Assets/Models/cube.obj", meshView);
	GRenderStageManager.RegisterStage("LightAccumulateStage", this);
}

void VolumetricLightStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto compManager = GContext->EntityManager->GetComponentManager();
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto rtManager = GContext->RenderTargetManager;
	auto sz = GPostProcess.GetPostSceneTextures().HalfResSize;
	uint32 count;
	const auto& camera = compManager->GetAllComponents<CameraComponent>(count)[0].CameraInstance;
	
	//auto projection = DirectX::XMLoadFloat4x4(&camera.Projection);
	//auto invProjection = DirectX::XMMatrixInverse(nullptr, projection);
	LightAccumParams params;
	params.InvView = camera.GetInverseViewTransposed();
	params.ScreenResolution = DirectX::XMUINT2((uint32)sz.Width, (uint32)sz.Height);
	//DirectX::XMStoreFloat4x4(&params.InvProjection, DirectX::XMMatrixTranspose(invProjection));
	params.InvProjection = camera.GetInverseProjectionTransposed();
	auto postProcessEntities = compManager->GetEntities<BaseDrawableComponent, PostProcessVolumeComponent>();
	auto baseDrawable = compManager->GetComponent<BaseDrawableComponent>(postProcessEntities[0]);
	params.World = baseDrawable->World;
	shaderResourceManager->CopyToCB(frameIndex, { &params, sizeof(params) }, lightAccumCBV);
	renderer->TransitionBarrier(commandList, intermediatelightAccumTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(intermediatelightAccumTarget.RenderTarget), ColorValues::ClearColor, 0, nullptr);
	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.LightAccumPSO));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)sz.Width;
	viewport.Height = (FLOAT)sz.Height;
	viewport.MaxDepth = 1.f;
	viewport.MinDepth = 0.f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = sz.Width;
	scissorRect.bottom = sz.Height;

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	renderer->SetRenderTargets(&intermediatelightAccumTarget.RenderTarget, 1, nullptr);

	TextureID textures[] = { GSceneResources.ShadowDepthTarget.Texture, GSceneResources.DepthPrePass.Texture, GSceneResources.NoiseTexture, GSceneResources.WorldPosTexture.Texture };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel2, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, GSceneResources.LightBufferCBV);
	renderer->SetConstantBufferView(commandList, RootSigCBVertex0, baseDrawable->CBView);
	renderer->SetConstantBufferView(commandList, RootSigCBAll1, GSceneResources.FrameDataCBV);
	renderer->SetConstantBufferView(commandList, RootSigCBAll2, lightAccumCBV);

	renderer->DrawScreenQuad(commandList);
	//renderer->DrawMesh(commandList, cubeMesh);

	renderer->TransitionBarrier(commandList, intermediatelightAccumTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	GPostProcess.RenderBlurTexture(intermediatelightAccumTarget.Texture, sz, frameIndex, blurFinalTarget,blurIntermediateTarget, blurVerticalCBV, blurHorizontalCBV);
	BilateralBlur(frameIndex);
}

void VolumetricLightStage::BilateralBlur(int frameIndex)
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto renderer = GContext->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto rtManager = GContext->RenderTargetManager;
	auto halfSize = GPostProcess.GetPostSceneTextures().HalfResSize;
	auto fullSize = renderer->GetScreenSize();

	BilateralBlurParams params;
	params.FullResolutionSize = DirectX::XMUINT2(fullSize.Width, fullSize.Height);
	params.LowResolutionSize = DirectX::XMUINT2(halfSize.Width, halfSize.Height);

	shaderResourceManager->CopyToCB(frameIndex, { &params, sizeof(params) }, bilateralBlurCBV);
	renderer->TransitionBarrier(commandList, lightAccumTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	commandList->ClearRenderTargetView(rtManager->GetRTVHandle(lightAccumTarget.RenderTarget), ColorValues::ClearColor, 0, nullptr);
	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.BilateralBlurPSO));

	D3D12_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)fullSize.Width;
	viewport.Height = (FLOAT)fullSize.Height;
	viewport.MaxDepth = 1.f;
	viewport.MinDepth = 0.f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = fullSize.Width;
	scissorRect.bottom = fullSize.Height;

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	renderer->SetRenderTargets(&lightAccumTarget.RenderTarget, 1, nullptr);

	TextureID textures[] = { GSceneResources.DepthPrePass.Texture, blurFinalTarget.Texture };
	renderer->SetShaderResourceViews(commandList, RootSigSRVPixel1, textures, _countof(textures));
	renderer->SetConstantBufferView(commandList, RootSigCBPixel0, bilateralBlurCBV);

	renderer->DrawScreenQuad(commandList);

	renderer->TransitionBarrier(commandList, lightAccumTarget.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
