#include "RenderTargetManager.h"


void RenderTargetManager::Initialize(ID3D12Device* device)
{
	this->device = device;
	rtvHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, CMaxRenderTargets);
	renderBuffers.reserve(CMaxRenderTargets);
}

RenderTargetID RenderTargetManager::CreateRenderTarget(ID3D12Resource* renderBuffer)
{
	RenderTargetID id = currentRtvIndex;
	currentRtvIndex++;
	D3D12_RENDER_TARGET_VIEW_DESC desc;
	ZeroMemory(&desc, sizeof(desc));
	desc.Texture2D.MipSlice = 0;
	desc.Texture2D.PlaneSlice = 0;
	desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	renderBuffers.push_back(renderBuffer);
	device->CreateRenderTargetView(renderBuffer, &desc, rtvHeap.handleCPU(id));
	return id;
}

D3D12_CPU_DESCRIPTOR_HANDLE RenderTargetManager::GetHandle(RenderTargetID rtvID)
{
	return rtvHeap.handleCPU(rtvID);
}
