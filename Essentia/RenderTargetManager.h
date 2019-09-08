#pragma once
#include "DXUtility.h"
#include "Declarations.h"
#include <d3d12.h>
#include <vector>

class RenderTargetManager
{
public:
	void								Initialize(ID3D12Device* device);
	RenderTargetID						CreateRenderTargetView(ID3D12Resource* renderBuffer, DXGI_FORMAT format);
	void 								ReCreateRenderTargetView(RenderTargetID renderTargetID, ID3D12Resource* resource, DXGI_FORMAT format);
	DepthStencilID						CreateDepthStencilView(ID3D12Resource* depthBuffer, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);
	void								ReCreateDepthStencilView(DepthStencilID dsId, ID3D12Resource* depthBuffer, DXGI_FORMAT format = DXGI_FORMAT_D32_FLOAT);
	D3D12_CPU_DESCRIPTOR_HANDLE			GetRTVHandle(RenderTargetID rtvID);
	D3D12_CPU_DESCRIPTOR_HANDLE			GetDSVHandle(DepthStencilID dsvID);
private:
	RenderTargetManager() {};

	RenderTargetID currentRtvIndex = 0;
	DepthStencilID currentDsvIndex = 0;
	DescriptorHeap rtvHeap;
	DescriptorHeap dsvHeap;

	ID3D12Device* device = nullptr;
	std::vector<ID3D12Resource*> renderBuffers;
	friend class Renderer;
};

