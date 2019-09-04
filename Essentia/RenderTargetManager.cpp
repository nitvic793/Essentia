#include "RenderTargetManager.h"


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
	renderBuffer->SetName(L"Render Target");
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
