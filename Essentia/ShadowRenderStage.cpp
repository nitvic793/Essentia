#include "pch.h"
#include "ShadowRenderStage.h"
#include "ResourceManager.h"
#include "EngineContext.h"
#include "RenderTargetManager.h"
#include "ShaderResourceManager.h"
#include "Renderer.h"
#include "Entity.h"
#include "ResourceManager.h"
#include "PipelineStates.h"
#include "SceneResources.h"

using namespace DirectX;

void ShadowRenderStage::Initialize()
{
	shadowCBVs = Vector<ConstantBufferView>(4);
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto renderer = GContext->RendererInstance;
	shadowDepthTarget = CreateDepthTarget(CShadowMapSize, CShadowMapSize, DXGI_FORMAT_D32_FLOAT, DXGI_FORMAT_R32_FLOAT);
	GSceneResources.ShadowDepthTarget = shadowDepthTarget;
	GSceneResources.ShadowCBV = shaderResourceManager->CreateCBV(sizeof(ShadowConstantBuffer));

	for (int i = 0; i < 4; ++i)
	{
		shadowCBVs.Push(shaderResourceManager->CreateCBV(sizeof(ShadowDirParams)));
	}

	GRenderStageManager.RegisterStage("ShadowRenderStage", this);
}

void ShadowRenderStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto renderer = GContext->RendererInstance;
	auto shaderResourceManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto rtManager = GContext->RenderTargetManager;
	auto commandList = renderer->GetDefaultCommandList();

	D3D12_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)CShadowMapSize;
	viewport.Height = (FLOAT)CShadowMapSize;
	viewport.MaxDepth = 1.f;
	viewport.MinDepth = 0.f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = CShadowMapSize;
	scissorRect.bottom = CShadowMapSize;

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	renderer->SetRenderTargets(nullptr, 0, &shadowDepthTarget.DepthStencil);

	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.ShadowDirPSO));

	Vector<ConstantBufferView> cbvStack((uint32)shadowCBVs.Size(), Mem::GetFrameAllocator());
	cbvStack.CopyFrom(shadowCBVs.GetData(), (uint32)shadowCBVs.Size());

	auto drawables = frameContext.Drawables;
	auto drawableCount = frameContext.DrawableCount;
	auto drawableModelCount = frameContext.DrawableModelCount;
	auto drawableModels = frameContext.DrawableModels;
	auto& worlds = frameContext.WorldMatrices;
	auto& modelWorlds = frameContext.ModelWorldMatrices;
	ShadowDirParams params = {};
	uint32 count = 0;
	auto lights = GContext->EntityManager->GetComponents<DirectionalLightComponent>(count);

	auto dir = -XMVector3Normalize(XMLoadFloat3(&lights[0].Direction));
	auto eyePosV = XMVectorSet(0, 0, 0, 0) + 100 * dir;
	XMFLOAT3 eyePos;
	XMStoreFloat3(&eyePos, eyePosV);

	//Create Shadow CBVs
	XMMATRIX shView = XMMatrixLookAtLH(
		eyePosV,	// Start back and in the air
		XMVectorSet(0, 0, 0, 0),	// Look at the origin
		XMVectorSet(0, 1, 0, 0));	// Up is up
	XMStoreFloat4x4(&params.View, XMMatrixTranspose(shView));

	XMMATRIX shProj = XMMatrixOrthographicLH(200.0f, 200.0f, frameContext.Camera->NearZ, frameContext.Camera->FarZ);
	XMStoreFloat4x4(&params.Projection, XMMatrixTranspose(shProj));

	ShadowConstantBuffer shadowCB = { params.View, params.Projection };
	shaderResourceManager->CopyToCB(frameIndex, { &shadowCB, sizeof(ShadowConstantBuffer) }, GSceneResources.ShadowCBV);

	renderer->TransitionBarrier(commandList, shadowDepthTarget.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE);
	commandList->ClearDepthStencilView(rtManager->GetDSVHandle(shadowDepthTarget.DepthStencil), D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
	for (size_t i = 0; i < drawableCount; ++i)
	{
		params.World = worlds[i];
		auto cbv = cbvStack.Pop();
		shaderResourceManager->CopyToCB(frameIndex, DataPack{ &params, sizeof(params) }, cbv);
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, cbv);
		renderer->DrawMesh(commandList, drawables[i].Mesh);
	}

	for (size_t i = 0; i < drawableModelCount; ++i)
	{
		params.World = modelWorlds[i];
		auto cbv = cbvStack.Pop();
		shaderResourceManager->CopyToCB(frameIndex, DataPack{ &params, sizeof(params) }, cbv);
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, cbv);
		auto model = GContext->ModelManager->GetModel(drawableModels[i].Model);
		renderer->DrawMesh(commandList, model.Mesh);
	}

	renderer->TransitionBarrier(commandList, shadowDepthTarget.Resource, D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}
