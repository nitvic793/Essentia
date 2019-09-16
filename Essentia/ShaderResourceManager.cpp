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
	for (uint32 i = 0; i < CFrameBufferCount; ++i)
	{
		gpuHeap[i].Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CFrameMaxDescriptorHeapCount, true);
		heapIndex[i] = 0;
	}
}

ID3D12DescriptorHeap* FrameManager::GetGPUDescriptorHeap(uint32 frameIndex) const
{
	return gpuHeap[frameIndex].pDescriptorHeap.Get();
}

void FrameManager::Reset(uint32 frameIndex)
{
	heapIndex[frameIndex] = 0;
}

uint32 FrameManager::Allocate(uint32 frameIndex, const DescriptorHeap& heap, uint32 numDescriptors, uint32 offset)
{
	auto index = heapIndex[frameIndex];
	device->CopyDescriptorsSimple(numDescriptors, gpuHeap[frameIndex].handleCPU(index), heap.handleCPU(offset), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	heapIndex[frameIndex] += numDescriptors;
	return index;
}

D3D12_GPU_DESCRIPTOR_HANDLE FrameManager::GetHandle(uint32 frameIndex, GPUHeapID index) const
{
	return gpuHeap[frameIndex].handleGPU(index);
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

	auto texIndex = textureCount;
	textureCount++;
	CreateShaderResourceView(device, *resource, textureHeap.handleCPU(texIndex), isCubeMap);
	textureMap[stringId] = texIndex;
	textureResources.push_back(*resource);
	return texIndex;
}

TextureID ShaderResourceManager::CreateTexture(ID3D12Resource* resource, bool isCubeMap, const char* name, DXGI_FORMAT format)
{
	StringID stringId;
	if (name == nullptr)
	{
		stringId = String::ID(std::to_string(textureCount).c_str());
	}
	else
	{
		stringId = String::ID(name);
	}

	if (textureMap.find(stringId) != textureMap.end())
	{
		return textureMap[stringId];
	}

	auto device = deviceResources->GetDevice();
	auto texIndex = textureCount;
	textureCount++;

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

	textureMap[stringId] = texIndex;
	textureResources.push_back(resource);
	return texIndex;
}

MaterialHandle ShaderResourceManager::CreateMaterial(TextureID* textures, uint32 textureCount, PipelineStateID psoID, Material& outMaterial, const char* name)
{
	StringID stringId;
	if (name == nullptr)
	{
		stringId = String::ID(std::to_string(materialCount).c_str());
	}
	else
	{
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

	outMaterial = Material{ materialCount, psoID, textureCount };
	materials.push_back(outMaterial);
	materialCount += textureCount;
	materialMap[stringId] = handle;
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

D3D12_GPU_DESCRIPTOR_HANDLE ShaderResourceManager::GetTextureGPUHandle(TextureID texID)
{
	return textureHeap.handleGPU(texID);
}

D3D12_CPU_DESCRIPTOR_HANDLE ShaderResourceManager::GetTextureCPUHandle(TextureID texID)
{
	return textureHeap.handleCPU(texID);
}
