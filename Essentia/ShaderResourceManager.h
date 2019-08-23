#pragma once

#include "DXUtility.h"
#include "Material.h"
#include "StringHash.h"
#include <unordered_map>

namespace Default
{
	constexpr TextureID			DefaultDiffuse = 1;
	constexpr TextureID			DefaultNormals = 2;
	constexpr TextureID			DefaultRoughness = 3;
	constexpr TextureID			DefaultMetalness = 4;
	constexpr PipelineStateID	DefaultMaterialPSO = 0;
}

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
	TextureID			CreateTexture(const std::string& filename, TextureType texType = WIC, bool generateMips = true);
	TextureID			CreateTexture(ID3D12Resource* resource, bool isCubeMap = false, const char* name = nullptr);
	MaterialHandle		CreateMaterial(TextureID* textures, uint32 textureCount, PipelineStateID psoID, Material& outMaterial, const char* name = nullptr);
	
	const Material&		GetMaterial(MaterialHandle handle);
	MaterialHandle		GetMaterialHandle(StringID material);
	TextureID			GetTexture(StringID texture);
	TextureID			RequestUninitializedTexture();
	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGPUHandle(TextureID texID);
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCPUHandle(TextureID texID);
private:
	GPUConstantBuffer		cbuffer[CFrameBufferCount];
	DescriptorHeap			cbvHeap[CFrameBufferCount];
	DescriptorHeap			textureHeap;
	DescriptorHeap			materialHeap;

	ResourceManager*		resourceManager = nullptr;
	DeviceResources*		deviceResources = nullptr;

	uint32					constantBufferCount = 0;
	uint32					textureCount = 0;
	uint32					materialCount = 0;
	uint64					currentCBufferOffset = 0;
	std::vector<Material>	materials;

	std::unordered_map<StringID, MaterialHandle>	materialMap;
	std::unordered_map<StringID, TextureID>			textureMap;
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

