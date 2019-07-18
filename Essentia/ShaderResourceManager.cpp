#include "pch.h"
#include "ShaderResourceManager.h"
#include "WICTextureLoader.h"
#include "DDSTextureLoader.h"
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

ID3D12DescriptorHeap*  FrameManager::GetGPUDescriptorHeap(uint32 frameIndex) const
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

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		textureHeap[i].Create(deviceResources->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CMaxTextureCount);
		cbvHeap[i].Create(deviceResources->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CMaxConstantBufferCount);
		cbuffer[i].Initialize(resourceManager, CMaxConstantBufferSize);
	}

	currentCBufferOffset = 0;
}

ConstantBufferView ShaderResourceManager::CreateCBV(uint32 sizeInBytes) 
{
	auto device = deviceResources->GetDevice();
	uint32 index = constantBufferCount;
	auto size = (sizeInBytes + AlignmentSize - 1) & ~(AlignmentSize - 1);
	constantBufferCount++;

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		D3D12_CONSTANT_BUFFER_VIEW_DESC	descBuffer;
		descBuffer.BufferLocation = cbuffer->GetAddress() + currentCBufferOffset;
		descBuffer.SizeInBytes = size;
		device->CreateConstantBufferView(&descBuffer, cbvHeap[i].handleCPU(index));
	}

	ConstantBufferView cbv = { currentCBufferOffset, index };
	currentCBufferOffset += size;
	return cbv;
}

void ShaderResourceManager::CopyToCB(uint32 frameIndex, const DataPack& data, uint64 offset)
{
	cbuffer[frameIndex].CopyData(data.Data, data.Size, offset);
}

GPUHeapOffsets ShaderResourceManager::CopyDescriptorsToGPUHeap(uint32 frameIndex, FrameManager* frame)
{
	GPUHeapOffsets offsets;
	offsets.ConstantBufferOffset = frame->Allocate(frameIndex, cbvHeap[frameIndex], constantBufferCount);
	//offsets.TexturesOffset = frame->Allocate(frameIndex, textureHeap[frameIndex], textureCount);

	return offsets;
}
