#include "pch.h"
#include "ShaderResourceManager.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
#include "ResourceUploadBatch.h"
#include "DeviceResources.h"
#include "ResourceManager.h"
#include <wrl.h>
#include "DirectXHelpers.h"
#include "EngineContext.h"
#include "Memory.h"

TextureID Default::DefaultDiffuse = 0;
TextureID Default::DefaultMetalness = 0;
TextureID Default::DefaultNormals = 0;
TextureID Default::DefaultRoughness = 0;
TextureID Default::DefaultMaterialPSO = 0;

using namespace Microsoft::WRL;
using namespace DirectX;
/// Frame Manager

void FrameManager::Initialize(ID3D12Device* device)
{
	this->device = device;
	//TODO: Remove loop to maintain only one GPU  side heap
	for (uint32 i = 0; i < 1; ++i)
	{
		gpuHeap[i].Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CFrameMaxDescriptorHeapCount * 3, true);
		heapIndex[i] = 0;
	}
}

ID3D12DescriptorHeap* FrameManager::GetGPUDescriptorHeap(uint32 frameIndex) const
{
	return gpuHeap[0].pDescriptorHeap.Get();
}

void FrameManager::Reset(uint32 frameIndex)
{
	heapIndex[0] = 0;
}

uint32 FrameManager::Allocate(uint32 frameIndex, const DescriptorHeap& heap, uint32 numDescriptors, uint32 offset)
{
	auto index = heapIndex[0];
	device->CopyDescriptorsSimple(numDescriptors, gpuHeap[0].handleCPU(index + CFrameMaxDescriptorHeapCount * frameIndex), heap.handleCPU(offset), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	heapIndex[0] += numDescriptors;
	return index;
}

D3D12_GPU_DESCRIPTOR_HANDLE FrameManager::GetHandle(uint32 frameIndex, GPUHeapID index) const
{
	return gpuHeap[0].handleGPU(index + CFrameMaxDescriptorHeapCount * frameIndex);
}

/// Shader Resource Manager

void ShaderResourceManager::Initialize(ResourceManager* resourceManager, DeviceResources* deviceResources)
{
	this->resourceManager = resourceManager;
	this->deviceResources = deviceResources;
	materials.reserve(CMaxTextureCount);
	materialHeap.Create(deviceResources->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CMaxTextureCount);
	textureHeap.Create(deviceResources->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CMaxTextureCount);
	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		cbvHeap[i].Create(deviceResources->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CMaxConstantBufferCount);
		cbuffer[i].Initialize(resourceManager, CMaxConstantBufferSize);
	}

	currentCBufferOffset = 0;
}

ConstantBufferView ShaderResourceManager::CreateCBV(uint32 sizeInBytes)
{
	auto device = deviceResources->GetDevice();
	uint32 index = constantBufferCount;
	uint32 size = (sizeInBytes + AlignmentSize - 1) & ~(AlignmentSize - 1);
	constantBufferCount++;

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
		descBuffer.BufferLocation = cbuffer[i].GetAddress() + currentCBufferOffset;
		descBuffer.SizeInBytes = size;
		device->CreateConstantBufferView(&descBuffer, cbvHeap[i].handleCPU(index));
	}

	ConstantBufferView cbv = { currentCBufferOffset, index };
	currentCBufferOffset += size;
	return cbv;
}

void ShaderResourceManager::CopyToCB(uint32 frameIndex, const DataPack& data, uint64 offset)
{
	//for (int i = 0; i < 3; ++i)
	cbuffer[frameIndex].CopyData(data.Data, data.Size, offset);
}

void ShaderResourceManager::CopyToCB(uint32 frameIndex, const DataPack& data, const ConstantBufferView& cbv)
{
	cbuffer[frameIndex].CopyData(data.Data, data.Size, cbv.Offset);
}

GPUHeapOffsets ShaderResourceManager::CopyDescriptorsToGPUHeap(uint32 frameIndex, FrameManager* frame)
{
	GPUHeapOffsets offsets;
	offsets.ConstantBufferOffset = frame->Allocate(frameIndex, cbvHeap[frameIndex], constantBufferCount);
	offsets.TexturesOffset = frame->Allocate(frameIndex, textureHeap, textureCount);
	offsets.MaterialsOffset = frame->Allocate(frameIndex, materialHeap, materialCount);
	return offsets;
}

TextureID ShaderResourceManager::CreateTexture(const std::string& filename, TextureType texType, bool generateMips)
{
	auto stringId = String::ID(filename.c_str());
	if (textureMap.find(stringId) != textureMap.end())
	{
		return textureMap[stringId];
	}

	auto device = deviceResources->GetDevice();
	ResourceUploadBatch uploadBatch(device);
	auto fname = ToWString(filename);
	ResourceID resourceId;
	ID3D12Resource** resource = resourceManager->RequestEmptyResource(resourceId);
	ComPtr<ID3D12Resource> tex;
	bool isCubeMap = false;

	uploadBatch.Begin();

	switch (texType)
	{
	case WIC:
		CreateWICTextureFromFile(device, uploadBatch, fname.c_str(), resource, generateMips);
		break;
	case DDS:
		CreateDDSTextureFromFile(device, uploadBatch, fname.c_str(), resource, generateMips, 0, nullptr, &isCubeMap);
		break;
	}

	auto finish = uploadBatch.End(deviceResources->GetCommandQueue());
	finish.wait();

	auto desc = (*resource)->GetDesc();

	TextureProperties properties = {};
	properties.HasMips = generateMips;
	properties.IsCubeMap = isCubeMap;
	properties.Height = desc.Height;
	properties.Width = (uint32)desc.Width;
	properties.Format = desc.Format;
	properties.Name = filename;
	properties.TextureLoadType = texType;

	auto texIndex = GetNextTextureIndex();
	CreateShaderResourceView(device, *resource, textureHeap.handleCPU(texIndex), isCubeMap);
	textureMap[stringId] = texIndex;
	textureResources.push_back(*resource);
	textureNameMap[texIndex] = filename;
	textureFiles.push_back(filename);
	texturePropertiesMap[texIndex] = properties;
	return texIndex;
}

TextureID ShaderResourceManager::CreateTexture(ID3D12Resource* resource, bool isCubeMap, const char* name, DXGI_FORMAT format)
{
	StringID stringId;
	std::string texName;
	if (name == nullptr)
	{
		texName = std::to_string(textureCount);
		stringId = String::ID(texName.c_str());
	}
	else
	{
		texName = name;
		stringId = String::ID(name);
	}

	if (textureMap.find(stringId) != textureMap.end())
	{
		return textureMap[stringId];
	}

	auto device = deviceResources->GetDevice();
	auto texIndex = GetNextTextureIndex();

	if (format == DXGI_FORMAT_UNKNOWN)
	{
		CreateShaderResourceView(device, resource, textureHeap.handleCPU(texIndex), isCubeMap);
	}
	else
	{
		auto viewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		if (isCubeMap) viewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.Texture2D.MipLevels = 1;
		descSRV.Texture2D.MostDetailedMip = 0;
		descSRV.Format = format;
		descSRV.ViewDimension = viewDimension;
		descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		device->CreateShaderResourceView(resource, &descSRV, textureHeap.handleCPU(texIndex));
	}

	resource->SetName(ToWString(texName).c_str());
	textureMap[stringId] = texIndex;
	textureResources.push_back(resource);
	textureNameMap[texIndex] = texName;
	return texIndex;
}

TextureID ShaderResourceManager::CreateTexture2D(const TextureCreateProperties& properties, ResourceID* outResourceId, const char* name, D3D12_RESOURCE_FLAGS flags, D3D12_RESOURCE_STATES initialState)
{
	auto ec = EngineContext::Context;
	auto resourceId = ec->ResourceManager->CreateResource(
		CD3DX12_RESOURCE_DESC::Tex2D(properties.Format, properties.Width, properties.Height, 1, 0, 1, 0, flags),
		nullptr, initialState);
	auto texResource = ec->ResourceManager->GetResource(resourceId);
	if (outResourceId != nullptr)
	{
		*outResourceId = resourceId;
	}
	return CreateTexture(texResource, false, nullptr, properties.Format);
}

TextureID ShaderResourceManager::CreateTexture3D(const Texture3DCreateProperties& properties,
	ResourceID* outResourceId,
	const char* name,
	D3D12_RESOURCE_FLAGS flags,
	D3D12_RESOURCE_STATES initialState)
{
	auto resourceId = GContext->ResourceManager->CreateResource(
		CD3DX12_RESOURCE_DESC::Tex3D(properties.Format, properties.Width, properties.Height, properties.Depth, properties.MipLevels, flags),
		nullptr, initialState);
	auto texResource = GContext->ResourceManager->GetResource(resourceId);
	if (outResourceId != nullptr)
	{
		*outResourceId = resourceId;
	}

	StringID stringId;
	std::string texName;
	if (name == nullptr)
	{
		texName = std::to_string(textureCount);
		stringId = String::ID(texName.c_str());
	}
	else
	{
		texName = name;
		stringId = String::ID(name);
	}

	if (textureMap.find(stringId) != textureMap.end())
	{
		return textureMap[stringId];
	}

	auto device = deviceResources->GetDevice();
	auto texIndex = GetNextTextureIndex();

	if (properties.Format == DXGI_FORMAT_UNKNOWN)
	{
		CreateShaderResourceView(device, texResource, textureHeap.handleCPU(texIndex));
	}
	else
	{
		auto viewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;

		D3D12_SHADER_RESOURCE_VIEW_DESC descSRV = {};
		ZeroMemory(&descSRV, sizeof(descSRV));
		descSRV.Texture3D.MipLevels = properties.MipLevels;
		descSRV.Texture3D.MostDetailedMip = 0;
		descSRV.Format = properties.Format;
		descSRV.ViewDimension = viewDimension;
		descSRV.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		device->CreateShaderResourceView(texResource, &descSRV, textureHeap.handleCPU(texIndex));
	}

	texResource->SetName(ToWString(texName).c_str());
	textureMap[stringId] = texIndex;
	textureResources.push_back(texResource);
	textureNameMap[texIndex] = texName;
	return texIndex;
}

TextureID ShaderResourceManager::CreateTextureUAV(ResourceID resourceId)
{
	auto texName = std::to_string(textureCount);
	auto stringId = String::ID(texName.c_str());
	auto resource = GContext->ResourceManager->GetResource(resourceId);
	auto rDesc = resource->GetDesc();
	auto texIndex = GetNextTextureIndex();

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
	desc.Format = rDesc.Format;
	deviceResources->GetDevice()->CreateUnorderedAccessView(resource, nullptr, &desc, textureHeap.handleCPU(texIndex));
	
	textureMap[stringId] = texIndex;
	textureResources.push_back(resource);
	textureNameMap[texIndex] = texName;
	return texIndex;
}

TextureID ShaderResourceManager::CreateTexture3DUAV(ResourceID resourceId, uint32 depthSlices, uint32 mipSlice)
{
	auto texName = std::to_string(textureCount);
	auto stringId = String::ID(texName.c_str());
	auto resource = GContext->ResourceManager->GetResource(resourceId);
	auto rDesc = resource->GetDesc();
	auto texIndex = GetNextTextureIndex();

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
	desc.Format = rDesc.Format;
	desc.Texture3D.MipSlice = mipSlice;
	desc.Texture3D.WSize = depthSlices;
	desc.Texture3D.FirstWSlice = 0;
	deviceResources->GetDevice()->CreateUnorderedAccessView(resource, nullptr, &desc, textureHeap.handleCPU(texIndex));
	
	textureMap[stringId] = texIndex;
	textureResources.push_back(resource);
	textureNameMap[texIndex] = texName;
	return texIndex;
}

TextureID ShaderResourceManager::CreateStructuredBufferUAV(ResourceID resourceId, uint32 stride)
{
	auto texName = std::to_string(textureCount);
	auto stringId = String::ID(texName.c_str());
	auto resource = GContext->ResourceManager->GetResource(resourceId);
	auto rDesc = resource->GetDesc();
	auto texIndex = GetNextTextureIndex();

	D3D12_UNORDERED_ACCESS_VIEW_DESC desc = {};
	desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	desc.Buffer.NumElements = (uint32)rDesc.Width / stride;
	desc.Buffer.StructureByteStride = stride;
	desc.Buffer.FirstElement = 0;
	deviceResources->GetDevice()->CreateUnorderedAccessView(resource, nullptr, &desc, textureHeap.handleCPU(texIndex));

	textureMap[stringId] = texIndex;
	textureResources.push_back(resource);
	textureNameMap[texIndex] = texName;
	return texIndex;
}

MaterialHandle ShaderResourceManager::CreateMaterial(TextureID* textures, uint32 textureCount, PipelineStateID psoID, Material& outMaterial, const char* name)
{
	StringID stringId;
	std::string matName;
	if (name == nullptr)
	{
		matName = std::to_string(materialCount);
		stringId = String::ID(matName.c_str());
	}
	else
	{
		matName = name;
		stringId = String::ID(name);
	}

	if (materialMap.find(stringId) != materialMap.end())
	{
		return materialMap[stringId];
	}

	auto device = deviceResources->GetDevice();
	MaterialHandle handle = { (uint32)materials.size() };
	//Copy Textures from texture heap to material heap so that material textures are ordered in descriptor table.
	for (auto i = 0u; i < textureCount; ++i)
	{
		device->CopyDescriptorsSimple(1, materialHeap.handleCPU(materialCount + i), textureHeap.handleCPU(textures[i]), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	MaterialData materialData;
	materialData.TextureCount = textureCount;
	materialData.Textures = Mem::AllocArray<TextureID>(textureCount);
	memcpy(materialData.Textures, textures, sizeof(TextureID) * textureCount);

	outMaterial = Material{ materialCount, psoID, textureCount };
	materials.push_back(outMaterial);
	materialDataList.push_back(materialData);
	materialCount += textureCount;
	materialMap[stringId] = handle;
	materialNameMap[handle.Index] = matName;
	return handle;
}

void ShaderResourceManager::CopyTexturesToHeap(TextureID* textures, uint32 textureCount, const DescriptorHeap& heap)
{
	auto device = deviceResources->GetDevice();
	for (uint32 i = 0; i < textureCount; ++i)
	{
		device->CopyDescriptorsSimple(1, textureHeap.handleCPU(textures[i]), heap.handleCPU(i), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceManager::AllocateTextures(TextureID* textures, uint32 textureCount, uint32 frameIndex, FrameManager* frameManager)
{
	assert(textureCount >= 1);
	auto device = deviceResources->GetDevice();
	uint32 firstIndex = frameManager->Allocate(frameIndex, textureHeap, 1, textures[0]);;
	for (uint32 i = 1; i < textureCount; ++i)
	{
		frameManager->Allocate(frameIndex, textureHeap, 1, textures[i]);
	}

	return frameManager->GetHandle(frameIndex, firstIndex);
}

const Material& ShaderResourceManager::GetMaterial(MaterialHandle handle)
{
	return materials[handle.Index];
}

MaterialHandle ShaderResourceManager::GetMaterialHandle(const char* materialName)
{
	auto stringId = String::ID(materialName);
	return GetMaterialHandle(stringId);
}

MaterialHandle ShaderResourceManager::GetMaterialHandle(StringID material)
{
	return materialMap[material];
}

TextureID ShaderResourceManager::GetTexture(StringID texture)
{
	return textureMap[texture];
}

TextureID ShaderResourceManager::RequestUninitializedTexture()
{
	TextureID texIndex = textureCount;
	textureCount++;
	return texIndex;
}

ID3D12Resource* ShaderResourceManager::GetResource(TextureID textureId)
{
	return textureResources[textureId];
}

std::string ShaderResourceManager::GetMaterialName(MaterialHandle handle)
{
	return materialNameMap[handle.Index];
}

std::string ShaderResourceManager::GetTextureName(TextureID textureId)
{
	return textureNameMap[textureId];
}

std::vector<std::string> ShaderResourceManager::GetAllMaterialNames()
{
	std::vector<std::string> materialNames;
	materialNames.reserve(materialNameMap.size());
	for(auto mat : materialNameMap)
	{	
		materialNames.push_back(mat.second);
	}

	return materialNames;
}

const std::vector<MaterialData>& ShaderResourceManager::GetAllMaterialData()
{
	return materialDataList;
}

std::vector<TextureProperties> ShaderResourceManager::GetAllTextures()
{
	std::vector<TextureProperties> properties;
	for (auto texProp : texturePropertiesMap)
	{
		properties.push_back(texProp.second);
	}

	return properties;
}

D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceManager::GetTextureGPUHandle(TextureID texID)
{
	return textureHeap.handleGPU(texID);
}

D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceManager::GetTextureCPUHandle(TextureID texID)
{
	return textureHeap.handleCPU(texID);
}

uint32 ShaderResourceManager::GetNextTextureIndex()
{
	auto texIndex = textureCount;
	textureCount++;
	return texIndex;
}
