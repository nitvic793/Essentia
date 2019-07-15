#pragma once
#include "DXUtility.h"
#include "Declarations.h"
#include <d3d12.h>
#include <vector>

typedef uint32 RenderTargetID;
const int CMaxRenderTargets = 32;

class RenderTargetManager
{
public:
	void								Initialize(ID3D12Device* device);
	RenderTargetID						CreateRenderTarget(ID3D12Resource* renderBuffer);
	D3D12_CPU_DESCRIPTOR_HANDLE	GetHandle(RenderTargetID rtvID);
private:
	RenderTargetManager() {};

	RenderTargetID currentRtvIndex = 0;
	CDescriptorHeapWrapper rtvHeap;
	ID3D12Device* device = nullptr;
	std::vector<ID3D12Resource*> renderBuffers;
	friend class Renderer;
};

