
#include "RenderTargetManager.h"
#include "EngineContext.h"
#include "ShaderResourceManager.h"

void RenderTargetManager::Initialize(ID3D12Device* device)
{
	this->device = device;
	rtvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, CMaxRenderTargets);
	dsvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, CMaxDepthStencils);
	renderBuffers.reserve(CMaxRenderTargets);
}

RenderTargetID RenderTargetManager::CreateRenderTargetView(ID3D12Resource* renderBuffer, DXGI_FORMAT format)
{
	RenderTargetID id = currentRtvIndex;
	currentRtvIndex++;

	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Texture2D.MipSlice = 0;
	desc.Texture2D.PlaneSlice = 0;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Format = format;

	renderBuffers.push_back(renderBuffer);
	renderBuffer->SetName(ToWString(std::to_string(id) + "Render Target").c_str());
	device->CreateRenderTargetView(renderBuffer, &desc, rtvHeap.handleCPU(id));
	return id;
}

void RenderTargetManager::ReCreateRenderTargetView(RenderTargetID renderTargetID, ID3D12Resource* resource, DXGI_FORMAT format)
{
	D3D12_RENDER_TARGET_VIEW_DESC desc = {};
	desc.Texture2D.MipSlice = 0;
	desc.Texture2D.PlaneSlice = 0;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
	desc.Format = format;
	device->CreateRenderTargetView(resource, &desc, rtvHeap.handleCPU(renderTargetID));
}

DepthStencilID RenderTargetManager::CreateDepthStencilView(ID3D12Resource* depthBuffer, DXGI_FORMAT format)
{
	DepthStencilID id = currentDsvIndex;
	currentDsvIndex++;

	D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
	desc.Texture2D.MipSlice = 0;
	desc.Format = format;
	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(depthBuffer, &desc, dsvHeap.handleCPU(id));
	return id;
}

void RenderTargetManager::ReCreateDepthStencilView(DepthStencilID dsId, ID3D12Resource* depthBuffer, DXGI_FORMAT format)
{
	D3D12_DEPTH_STENCIL_VIEW_DESC desc = {};
	desc.Texture2D.MipSlice = 0;
	desc.Format = format;
	desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	desc.Flags = D3D12_DSV_FLAG_NONE;

	device->CreateDepthStencilView(depthBuffer, &desc, dsvHeap.handleCPU(dsId));
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetManager::GetRTVHandle(RenderTargetID rtvID)
{
	return rtvHeap.handleCPU(rtvID);
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetManager::GetDSVHandle(DepthStencilID dsvID)
{
	return dsvHeap.handleCPU(dsvID);
}

SceneRenderTarget CreateSceneRenderTarget(EngineContext* context, uint32 width, uint32 height, DXGI_FORMAT format)
{
	SceneRenderTarget target;
	auto ec = context;
	target.Texture = ec->ShaderResourceManager->CreateTexture2D({ width, height, format }, &target.Resource);
	auto resource = ec->ShaderResourceManager->GetResource(target.Texture);
	target.RenderTarget = ec->RenderTargetManager->CreateRenderTargetView(resource, format);
	return target;
}

DepthTarget CreateDepthTarget(uint32 width, uint32 height, DXGI_FORMAT depthFormat, DXGI_FORMAT depthTextureFormat, D3D12_RESOURCE_STATES initialState)
{
	auto resourceManager = GContext->ResourceManager;
	auto renderTargetManager = GContext->RenderTargetManager;
	auto shaderResourceManager = GContext->ShaderResourceManager;

	auto clearVal = CD3DX12_CLEAR_VALUE(depthFormat, 1.f, 0);
	auto depthBufferResourceId = resourceManager->CreateResource(
		CD3DX12_RESOURCE_DESC::Tex2D(depthFormat, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		&clearVal,
		initialState
	);

	auto depthBuffer = resourceManager->GetResource(depthBufferResourceId);
	auto depthStencilId = renderTargetManager->CreateDepthStencilView(depthBuffer, depthFormat);
	auto depthStencilTexture = shaderResourceManager->CreateTexture(depthBuffer, false, nullptr, depthTextureFormat);

	return DepthTarget{ depthStencilTexture, depthBufferResourceId,  depthStencilId };
}
