#include "pch.h"
#include "CommandContext.h"

void CommandContext::Initialize(DeviceResources* deviceResources)
{
	this->deviceResources = deviceResources;
	auto commandQueue = deviceResources->GetCommandQueue();
	auto device = deviceResources->GetDevice();

	HRESULT hr;
	for (int i = 0; i < CFrameBufferCount; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocators[i].ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			throw std::runtime_error("Unable to create command allocators");
		}
	}

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), NULL, IID_PPV_ARGS(commandList.ReleaseAndGetAddressOf()));

	for (int i = 0; i < CFrameBufferCount; i++)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fences[i].ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			throw std::runtime_error("Unable to create fences.");
		}
		fenceValues[i] = i == 0 ? 1 : 0; // This is to ensure that the fence is set when 
		fenceEvent[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
}

ID3D12CommandAllocator* CommandContext::GetAllocator(uint32 index)
{
	return commandAllocators[index].Get();
}

ID3D12GraphicsCommandList* CommandContext::GetDefaultCommandList()
{
	return commandList.Get();
}

ID3D12Device* CommandContext::GetDevice()
{
	return deviceResources->GetDevice();
}

void CommandContext::SubmitCommands(ID3D12GraphicsCommandList* commandList)
{
	auto commandQueue = deviceResources->GetCommandQueue();
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	fenceValues[backBufferIndex]++;
	auto hr = commandQueue->Signal(fences[backBufferIndex].Get(), fenceValues[backBufferIndex]);
}

void CommandContext::WaitForFrame()
{
	auto swapChain = deviceResources->GetSwapChain();
	auto commandQueue = deviceResources->GetCommandQueue();
	const UINT64 currentFenceValue = fenceValues[backBufferIndex];

	auto hr = commandQueue->Signal(fences[backBufferIndex].Get(), currentFenceValue);
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	if (fences[backBufferIndex]->GetCompletedValue() < fenceValues[backBufferIndex])
	{
		auto hr = fences[backBufferIndex]->SetEventOnCompletion(fenceValues[backBufferIndex], fenceEvent[backBufferIndex]);
		WaitForSingleObjectEx(fenceEvent[backBufferIndex], INFINITE, FALSE);
	}

	fenceValues[backBufferIndex] = currentFenceValue + 1;
}

void CommandContext::CleanUp()
{
	auto commandQueue = deviceResources->GetCommandQueue();
	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		uint32 backBufferIndex = i;
		WaitForFrame(backBufferIndex);
		commandQueue->Signal(fences[backBufferIndex].Get(), fenceValues[backBufferIndex]);
		CloseHandle(fenceEvent[i]);
	}
}

void CommandContext::ResetAllocator(ID3D12CommandAllocator* allocator)
{
	auto hr = allocator->Reset();
	if (FAILED(hr))
	{
	}
}

void CommandContext::ResetCommandList(ID3D12GraphicsCommandList* commandList, ID3D12CommandAllocator* allocator)
{
	auto hr = commandList->Reset(allocator, nullptr);
	if (FAILED(hr))
	{
	}
}

void CommandContext::CreateAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator** allocator)
{
	auto device = deviceResources->GetDevice();
	device->CreateCommandAllocator(type, IID_PPV_ARGS(allocator));
}

uint64& CommandContext::FenceValue(int index)
{
	return fenceValues[index];
}

HANDLE CommandContext::GetFenceEvent()
{
	return fenceEvent[backBufferIndex];
}

ID3D12Fence* CommandContext::GetFence()
{
	return fences[backBufferIndex].Get();
}

void CommandContext::WaitForFrame(uint32 index)
{
	auto commandQueue = deviceResources->GetCommandQueue();
	backBufferIndex = index;
	if (fences[index]->GetCompletedValue() < fenceValues[index])
	{
		auto hr = fences[index]->SetEventOnCompletion(fenceValues[index], fenceEvent[index]);
		if (SUCCEEDED(hr))
		{

		}
		if (WaitForSingleObjectEx(fenceEvent[index], 100, FALSE) == WAIT_TIMEOUT)
		{
			WaitForGPU(index);
		}
	}

	fenceValues[index]++;
}

void CommandContext::WaitForGPU(uint32 backBufferIndex)
{
	auto commandQueue = deviceResources->GetCommandQueue();

	// Signal command queue to update fence to given fence value. 
	if (SUCCEEDED(commandQueue->Signal(fences[backBufferIndex].Get(), fenceValues[backBufferIndex])))
	{
		auto hr = fences[backBufferIndex]->SetEventOnCompletion(fenceValues[backBufferIndex], fenceEvent[backBufferIndex]);
		if (SUCCEEDED(hr))
		{
			WaitForSingleObjectEx(fenceEvent[backBufferIndex], INFINITE, FALSE);
		}
	}
	fenceValues[backBufferIndex]++;
}
