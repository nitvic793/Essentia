#include "pch.h"
#include "ShaderResourceManager.h"

void FrameManager::Initialize(ID3D12Device* device)
{
	this->device = device;
	for (uint32 i = 0; i < CFrameBufferCount; ++i)
	{
		gpuHeap[i].Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, CFrameMaxDescriptorHeapCount, true);
	}
}

ID3D12DescriptorHeap* FrameManager::GetGPUDescriptorHeap(uint32 frameIndex) const
{
	return gpuHeap[frameIndex].pDescriptorHeap.Get();
}
