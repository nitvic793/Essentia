#include "pch.h"
#include "VoxelizationStage.h"
#include "Renderer.h"
#include "EngineContext.h"
#include "Entity.h"
#include "PipelineStates.h"
#include "SceneResources.h"
#include <DirectXCollision.h>
using namespace DirectX;

constexpr uint32 CVoxelSize = 128;


struct DECLSPEC_ALIGN(16) VoxelParams
{
	XMFLOAT3 VoxelRCPSize;
	float Padding;
	XMFLOAT3 VoxelGridMaxPoint;
	float Padding2;
	XMFLOAT3 VoxelGridMinPoint;
	float Padding3;
	XMFLOAT3 VoxelGridCenter;
	float Padding4;
	XMFLOAT3 VoxelGridSize;
	float Padding5;

	XMFLOAT4X4 VoxelGridViewProjMatrices[3];
};

//Reference: https://github.com/KolyaNaichuk/RenderSDK/blob/c176de53d1f0c08f7e5be6790229203da305bc65/Samples/DynamicGI/Source/DXApplication.cpp

static const VoxelParams CreateVoxelParams(const Camera& camera, uint32 voxelSize)
{
	VoxelParams output = {};
	XMFLOAT3 corners[BoundingFrustum::CORNER_COUNT];
	camera.Frustum.GetCorners(corners);
	BoundingBox cameraAABB;
	BoundingBox::CreateFromPoints(cameraAABB, camera.Frustum.CORNER_COUNT, corners, sizeof(XMFLOAT3));

	XMVECTOR voxelGridCenter = XMLoadFloat3(&cameraAABB.Center);
	XMVECTOR voxelWorldRadius = XMVectorSet(camera.FarZ, camera.FarZ, camera.FarZ, 0.f);
	XMVECTOR numVoxelsInGrid = XMVectorSet(CVoxelSize, CVoxelSize, CVoxelSize, 0.f);

	XMVECTOR up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR right = XMVectorSet(1, 0, 0, 0);
	XMVECTOR forward = XMVectorSet(0, 0, 1, 0);

	XMVECTOR voxelWorldGridSize = 2.f * voxelWorldRadius;

	XMStoreFloat3(&output.VoxelGridMinPoint, voxelGridCenter - voxelWorldRadius);
	XMStoreFloat3(&output.VoxelGridMaxPoint, voxelGridCenter + voxelWorldRadius);
	XMStoreFloat3(&output.VoxelRCPSize, numVoxelsInGrid / voxelWorldGridSize);
	XMStoreFloat3(&output.VoxelGridCenter, voxelGridCenter);
	XMStoreFloat3(&output.VoxelGridSize, voxelWorldGridSize);


	Camera voxelGridCameraAlongX(CVoxelSize, CVoxelSize, 0.001f, CVoxelSize);
	voxelGridCameraAlongX.IsOrthographic = true;
	XMStoreFloat3(&voxelGridCameraAlongX.Position, voxelGridCenter - voxelWorldRadius * right);
	voxelGridCameraAlongX.Rotation.y = XM_PIDIV2;
	voxelGridCameraAlongX.Update();

	Camera voxelGridCameraAlongY(CVoxelSize, CVoxelSize, 0.001f, CVoxelSize);
	voxelGridCameraAlongY.IsOrthographic = true;
	XMStoreFloat3(&voxelGridCameraAlongY.Position, voxelGridCenter - voxelWorldRadius * up);
	voxelGridCameraAlongY.Rotation.y = -XM_PIDIV2;
	voxelGridCameraAlongY.Update();

	Camera voxelGridCameraAlongZ(CVoxelSize, CVoxelSize, 0.001f, CVoxelSize);
	voxelGridCameraAlongZ.IsOrthographic = true;
	XMStoreFloat3(&voxelGridCameraAlongZ.Position, voxelGridCenter - voxelWorldRadius * forward);
	voxelGridCameraAlongZ.Update();

	XMStoreFloat4x4(&output.VoxelGridViewProjMatrices[0], XMLoadFloat4x4(&voxelGridCameraAlongX.View) * XMLoadFloat4x4(&voxelGridCameraAlongX.Projection));
	XMStoreFloat4x4(&output.VoxelGridViewProjMatrices[1], XMLoadFloat4x4(&voxelGridCameraAlongY.View) * XMLoadFloat4x4(&voxelGridCameraAlongY.Projection));
	XMStoreFloat4x4(&output.VoxelGridViewProjMatrices[2], XMLoadFloat4x4(&voxelGridCameraAlongZ.View) * XMLoadFloat4x4(&voxelGridCameraAlongZ.Projection));

	return output;
}

void VoxelizationStage::Initialize()
{
	auto shaderResourceManager = GContext->ShaderResourceManager;
	uint32 voxelGridSize = CVoxelSize;
	Texture3DCreateProperties props = {};
	auto renderer = GContext->RendererInstance;
	props.Format = DXGI_FORMAT_R16G16B16A16_UNORM; //DXGI_FORMAT_R16G16B16A16_FLOAT; 
	props.Width = voxelGridSize;
	props.Depth = voxelGridSize;
	props.Height = voxelGridSize;
	props.MipLevels = 1;

	voxelGrid3dTextureSRV = shaderResourceManager->CreateTexture3D(props, &voxelGridResource, nullptr, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS);
	voxelGrid3dTextureUAV = shaderResourceManager->CreateTexture3DUAV(voxelGridResource, voxelGridSize);
	voxelRT = CreateSceneRenderTarget(GContext, renderer->GetScreenSize().Width, renderer->GetScreenSize().Width, DXGI_FORMAT_R8G8B8A8_UNORM);
	voxelParamsCBV = shaderResourceManager->CreateCBV(sizeof(VoxelParams));
	renderer->TransitionBarrier(renderer->GetDefaultCommandList(), voxelRT.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);
	GRenderStageManager.RegisterStage("VoxelizationStage", this);
}

void VoxelizationStage::Render(const uint32 frameIndex, const FrameContext& frameContext)
{
	auto ec = EngineContext::Context;
	auto renderer = ec->RendererInstance;
	auto commandList = renderer->GetDefaultCommandList();
	auto modelManager = ec->ModelManager;
	auto sz = renderer->GetScreenSize();
	auto rtManager = GContext->RenderTargetManager;
	auto srManager = GContext->ShaderResourceManager;
	auto resourceManager = GContext->ResourceManager;
	auto shaderResourceManager = GContext->ShaderResourceManager;
	uint32 count = 0;
	auto cameras = GContext->EntityManager->GetComponents<CameraComponent>(count);

	auto voxelParams = CreateVoxelParams(cameras[0].CameraInstance, CVoxelSize);
	shaderResourceManager->CopyToCB(frameIndex, { &voxelParams, sizeof(VoxelParams) }, voxelParamsCBV);

	D3D12_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)CVoxelSize;
	viewport.Height = (FLOAT)CVoxelSize;
	viewport.MaxDepth = 1.f;
	viewport.MinDepth = 0.f;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = sz.Width;
	scissorRect.bottom = sz.Height;

	TransitionDesc transitions[] = {
		{ voxelGridResource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_UNORDERED_ACCESS }
	};

	renderer->TransitionBarrier(commandList, transitions, _countof(transitions));

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	renderer->SetRenderTargets(nullptr, 0, nullptr);
	renderer->SetPipelineState(commandList, GPipelineStates.VoxelizePSO);
	uint32 clearVal[] = { 0, 0, 0, 0 };
	const FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearUnorderedAccessViewFloat(
		renderer->GetTextureGPUHandle(voxelGrid3dTextureUAV),
		srManager->GetTextureCPUHandle(voxelGrid3dTextureUAV),
		resourceManager->GetResource(voxelGridResource), clearColor, 0, nullptr);

	renderer->SetConstantBufferView(commandList, RootSigCBAll1, GSceneResources.ShadowCBV);
	renderer->SetConstantBufferView(commandList, RootSigCBAll2, voxelParamsCBV);
	renderer->SetShaderResourceView(commandList, RootSigUAV0, voxelGrid3dTextureUAV);

	auto drawables = frameContext.EntityManager->GetComponents<DrawableComponent>(count);

	for (uint32 i = 0; i < count; ++i)
	{
		auto mesh = drawables[i].Mesh;
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawables[i].CBView);
		renderer->SetShaderResourceViewMaterial(commandList, RootSigSRVPixel1, drawables[i].Material);
		renderer->DrawMesh(commandList, mesh);
	}

	auto drawableModels = frameContext.EntityManager->GetComponents<DrawableModelComponent>(count);
	for (uint32 i = 0; i < count; ++i)
	{
		auto& model = modelManager->GetModel(drawableModels[i].Model);
		renderer->SetConstantBufferView(commandList, RootSigCBVertex0, drawableModels[i].CBView);
		auto meshHandle = model.Mesh;
		renderer->DrawMesh(commandList, meshHandle);
	}

	TransitionDesc endTransitions[] = {
		{ voxelGridResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS,  D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE }
	};

	renderer->TransitionBarrier(commandList, endTransitions, _countof(endTransitions));
}


