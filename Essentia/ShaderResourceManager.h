#pragma once

#include "DXUtility.h"
#include "Material.h"

class DeviceResources;
class ResourceManager;
class FrameManager;

class ShaderResourceManager
{
public:
	void				Initialize(ResourceManager* resourceManager, DeviceResources* deviceResources);
	ConstantBufferView	CreateCBV(uint32 sizeInBytes);
	void				CopyToCB(uint32 frameIndex, const DataPack& data, uint64 offset = 0); //Copy data to constant buffer
	GPUHeapOffsets		CopyDescriptorsToGPUHeap(uint32 frameIndex, FrameManager* frame);
	TextureID			CreateTexture(const std::string& filename, TextureType texType = WIC);
	Material			CreateMaterial(TextureID* textures, uint32 textureCount, PipelineStateID psoID);
private:
	GPUConstantBuffer		cbuffer[CFrameBufferCount];
	DescriptorHeap			cbvHeap[CFrameBufferCount];
	DescriptorHeap			textureHeap[CFrameBufferCount];
	DescriptorHeap			materialHeap;

	ResourceManager*		resourceManager = nullptr;
	DeviceResources*		deviceResources = nullptr;

	uint32					constantBufferCount = 0;
	uint32					textureCount = 0;
	uint32					materialCount = 0;
	uint64					currentCBufferOffset = 0;

	friend class Renderer;
};

class FrameManager
{
public:
	void						Initialize(ID3D12Device* device);
	ID3D12DescriptorHeap*		GetGPUDescriptorHeap(uint32 frameIndex) const;
	void						Reset(uint32 frameIndex);
	uint32						Allocate(uint32 frameIndex, const DescriptorHeap& heap, uint32 numDescriptors, uint32 offset = 0);
	D3D12_GPU_DESCRIPTOR_HANDLE GetHandle(uint32 frameIndex, GPUHeapID index) const;
private:
	FrameManager() {};
	ID3D12Device*		device = nullptr;
	DescriptorHeap		gpuHeap[CFrameBufferCount];
	uint32				heapIndex[CFrameBufferCount] = { 0 };
	friend class Renderer;
};

