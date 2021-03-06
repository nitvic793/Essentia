#pragma once

#include "DXUtility.h"
#include "Material.h"
#include "StringHash.h"
#include <unordered_map>

struct Default
{
	static TextureID			DefaultDiffuse;
	static TextureID			DefaultNormals;
	static TextureID			DefaultRoughness;
	static TextureID			DefaultMetalness;
	static PipelineStateID		DefaultMaterialPSO;
};

class DeviceResources;
class ResourceManager;
class FrameManager;

struct TextureCreateProperties
{
	uint32		Width;
	uint32		Height;
	DXGI_FORMAT Format;
};

struct Texture3DCreateProperties
{
	uint64 Width;
	uint32 Height;
	uint16 Depth;
	DXGI_FORMAT Format;
	uint16 MipLevels;
};

struct TextureProperties
{
	std::string Name;
	uint32		Width;
	uint32		Height;
	DXGI_FORMAT Format;
	bool		IsCubeMap;
	bool		HasMips;
	TextureType	TextureLoadType;

	template<typename Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(Name),
			CEREAL_NVP(Width),
			CEREAL_NVP(Height),
			CEREAL_NVP(Format),
			CEREAL_NVP(IsCubeMap),
			CEREAL_NVP(HasMips),
			CEREAL_NVP(TextureLoadType)
		);
	}
};

class ShaderResourceManager
{
public:
	void						Initialize(ResourceManager* resourceManager, DeviceResources* deviceResources);
	ConstantBufferView			CreateCBV(uint32 sizeInBytes);
	void						CopyToCB(uint32 frameIndex, const DataPack& data, uint64 offset = 0); //Copy data to constant buffer
	void						CopyToCB(uint32 frameIndex, const DataPack& data, const ConstantBufferView& cbv); //Copy data to constant buffer
	GPUHeapOffsets				CopyDescriptorsToGPUHeap(uint32 frameIndex, FrameManager* frame);
	TextureID					CreateTexture(const std::string& filename, TextureType texType = WIC, bool generateMips = true);
	TextureID					CreateTexture(ID3D12Resource* resource, bool isCubeMap = false, const char* name = nullptr, DXGI_FORMAT format = DXGI_FORMAT_UNKNOWN);
	TextureID					CreateTexture2D(const TextureCreateProperties& properties,
		ResourceID* outResourceId = nullptr,
		const char* name = nullptr,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET,
		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	TextureID					CreateTexture3D(const Texture3DCreateProperties& properties,
		ResourceID* outResourceId = nullptr,
		const char* name = nullptr,
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE,
		D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	TextureID					CreateTextureUAV(ResourceID resourceId);
	TextureID					CreateTexture3DUAV(ResourceID resourceId, uint32 depthSlices = 1, uint32 mipSlice = 0);
	TextureID					CreateStructuredBufferUAV(ResourceID resourceId, uint32 stride);
	MaterialHandle				CreateMaterial(TextureID* textures, uint32 textureCount, PipelineStateID psoID, Material& outMaterial, const char* name = nullptr);

	void						CopyTexturesToHeap(TextureID* textures, uint32 textureCount, const DescriptorHeap& heap);
	D3D12_GPU_DESCRIPTOR_HANDLE	AllocateTextures(TextureID* textures, uint32 textureCount, uint32 frameIndex, FrameManager* frameManager);

	const Material& GetMaterial(MaterialHandle handle);
	MaterialHandle				GetMaterialHandle(const char* materialName);
	MaterialHandle				GetMaterialHandle(StringID material);
	TextureID					GetTexture(StringID texture);
	TextureID					RequestUninitializedTexture();
	ID3D12Resource* GetResource(TextureID textureId);
	std::string					GetMaterialName(MaterialHandle handle);
	std::string					GetTextureName(TextureID textureId);
	std::vector<std::string>	GetAllMaterialNames();

	const std::vector<MaterialData>&	GetAllMaterialData();
	//Will only return textures created via files.
	std::vector<TextureProperties>		GetAllTextures();

	D3D12_GPU_DESCRIPTOR_HANDLE GetTextureGPUHandle(TextureID texID);
	D3D12_CPU_DESCRIPTOR_HANDLE GetTextureCPUHandle(TextureID texID);
private:
	uint32					GetNextTextureIndex();
	GPUConstantBuffer		cbuffer[CFrameBufferCount];
	DescriptorHeap			cbvHeap[CFrameBufferCount];
	DescriptorHeap			textureHeap;
	DescriptorHeap			materialHeap;

	ResourceManager* resourceManager = nullptr;
	DeviceResources* deviceResources = nullptr;

	uint32					constantBufferCount = 0;
	uint32					textureCount = 0;
	uint32					materialCount = 0;
	uint64					currentCBufferOffset = 0;
	std::vector<Material>	materials;

	std::vector<MaterialData>						materialDataList;
	std::unordered_map<uint32, std::string>			materialNameMap;
	std::unordered_map<uint32, std::string>			textureNameMap;
	std::vector<std::string>						textureFiles;
	std::unordered_map<StringID, MaterialHandle>	materialMap;
	std::unordered_map<StringID, TextureID>			textureMap;
	std::unordered_map<StringID, TextureProperties> texturePropertiesMap;
	std::vector<ID3D12Resource*>					textureResources;
	friend class Renderer;
};

class FrameManager
{
public:
	void						Initialize(ID3D12Device* device);
	ID3D12DescriptorHeap* GetGPUDescriptorHeap(uint32 frameIndex) const;
	void						Reset(uint32 frameIndex);
	uint32						Allocate(uint32 frameIndex, const DescriptorHeap& heap, uint32 numDescriptors, uint32 offset = 0);
	D3D12_GPU_DESCRIPTOR_HANDLE GetHandle(uint32 frameIndex, GPUHeapID index) const;
private:
	FrameManager() {};
	ID3D12Device* device = nullptr;
	DescriptorHeap		gpuHeap[CFrameBufferCount];
	uint32				heapIndex[CFrameBufferCount] = { 0 };
	friend class Renderer;
};

