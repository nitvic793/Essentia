#pragma once

#include "DXUtility.h"
#include "ResourceManager.h"
#include "DeviceResources.h"

constexpr uint32 CFrameMaxDescriptorHeapCount = 512;

class ShaderResourceManager
{
public:


private:
	GPUConstantBuffer	perObjectBuffer;
	DescriptorHeap		cbvHeap;
	DescriptorHeap		textureHeap;
	ResourceManager*	resourceManager;
	DeviceResources*	deviceResources;
	friend class Renderer;
};

class FrameManager
{
public:
	void					Initialize(ID3D12Device* device);
	ID3D12DescriptorHeap*	GetGPUDescriptorHeap(uint32 frameIndex) const;
private:
	ID3D12Device*		device;
	DescriptorHeap		gpuHeap[CFrameBufferCount];
};

