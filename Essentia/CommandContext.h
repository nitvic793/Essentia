#pragma once

#include "Declarations.h"
#include "d3dx12.h"
#include "DeviceResources.h"

class CommandContext
{
public:
	void							Initialize(DeviceResources* deviceResources);
	ID3D12CommandAllocator*			GetAllocator(uint32 index);
	ID3D12GraphicsCommandList*		GetDefaultCommandList();
	ID3D12Device*					GetDevice();
	void							SubmitCommands(ID3D12GraphicsCommandList* commandList);
	void							WaitForFrame();
	void							WaitForFrame(uint32 backBufferIndex);
	void							CleanUp();
	void							ResetAllocator(ID3D12CommandAllocator* allocator);
	void							ResetCommandList(ID3D12GraphicsCommandList* commandList, ID3D12CommandAllocator* allocator);
	void							CreateAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator** allocator);
private:
	uint32												backBufferIndex;
	DeviceResources*									deviceResources;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>		commandAllocators[CFrameBufferCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>	commandList;
	Microsoft::WRL::ComPtr<ID3D12Fence>					fences[CFrameBufferCount];
	uint64												fenceValues[CFrameBufferCount];
	HANDLE												fenceEvent;
};

