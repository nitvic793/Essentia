#pragma once

#include "DXUtility.h"
#include "ResourceManager.h"
#include "DeviceResources.h"

constexpr uint32 CFrameMaxDescriptorHeapCount = 512;
constexpr uint32 CMaxTextureCount = 512;
constexpr uint32 CMaxConstantBufferCount = 512;
constexpr uint64 CMaxConstantBufferSize = 1024 * 4; //4KB

typedef uint32 GPUHeapID;

struct ConstantBufferView
{
	uint64		Offset;
	GPUHeapID	Index;
};

struct DataPack
{
	void*	Data;
	uint32	Size;
};

struct GPUHeapOffsets
{
	uint32 ConstantBufferOffset;
	uint32 TexturesOffset;
};

enum TextureType
{
	WIC,
	DDS
};

class FrameManager;

class ShaderResourceManager
{
public:
	void				Initialize(ResourceManager* resourceManager, DeviceResources* deviceResources);
	ConstantBufferView	CreateCBV(uint32 sizeInBytes);
	void				CopyToCB(uint32 frameIndex, const DataPack& data, uint64 offset = 0); //Copy data to constant buffer
	GPUHeapOffsets		CopyDescriptorsToGPUHeap(uint32 frameIndex, FrameManager* frame);
	GPUHeapID			CreateTexture(const std::string& filename, TextureType texType = WIC);
private:
	GPUConstantBuffer		cbuffer[CFrameBufferCount];
	DescriptorHeap			cbvHeap[CFrameBufferCount];
	DescriptorHeap			textureHeap[CFrameBufferCount];

	ResourceManager*		resourceManager = nullptr;
	DeviceResources*		deviceResources = nullptr;

	uint32					constantBufferCount = 0;
	uint32					textureCount = 0;
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

