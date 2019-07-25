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
		fenceValues[i] = 0;
	}

	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
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
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);;
	fenceValues[backBufferIndex]++;
	auto hr = commandQueue->Signal(fences[backBufferIndex].Get(), fenceValues[backBufferIndex]);
}

void CommandContext::WaitForFrame()
{
	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	WaitForFrame(backBufferIndex);
}

void CommandContext::CleanUp()
{
	auto commandQueue = deviceResources->GetCommandQueue();
	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		uint32 backBufferIndex = i;
		WaitForFrame(backBufferIndex);
		commandQueue->Signal(fences[backBufferIndex].Get(), fenceValues[backBufferIndex]);
	}

	CloseHandle(fenceEvent);
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

void CommandContext::WaitForFrame(uint32 index)
{
	backBufferIndex = index;
	if (fences[index]->GetCompletedValue() < fenceValues[index])
	{
		auto hr = fences[index]->SetEventOnCompletion(fenceValues[index], fenceEvent);
		if (FAILED(hr))
		{
		}
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	fenceValues[index]++;
}
