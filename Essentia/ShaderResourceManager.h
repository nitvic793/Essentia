#pragma once

#include "DXUtility.h"
#include "ResourceManager.h"

class ShaderResourceManager
{
public:


private:
	ConstantBuffer		perObjectBuffer;
	DescriptorHeap		cbvHeap;
	DescriptorHeap		textureHeap;
	ResourceManager*	resourceManager;

	friend class Renderer;
};

class FrameManager
{
public:
	ID3D12DescriptorHeap* GetGPUDescriptorHeap() const
	{
		return gpuHeap.pDescriptorHeap.Get();
	}
private:
	DescriptorHeap		gpuHeap;
};

