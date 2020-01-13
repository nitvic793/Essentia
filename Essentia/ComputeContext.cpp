#include "pch.h"
#include "ComputeContext.h"
#include "EngineContext.h"

RootSignatureID CreateComputeRootSignature()
{
	auto resourceManager = GContext->ResourceManager;

	CD3DX12_DESCRIPTOR_RANGE range[3];
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[3];
	rootParameters[0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[2].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(3, rootParameters, 0, nullptr);

	return resourceManager->CreateRootSignature(descRootSignature);
}

void ComputeContext::Initialize(DeviceResources* deviceResources)
{
	this->deviceResources = deviceResources;
	auto commandQueue = deviceResources->GetComputeQueue(); //ComputeQueue
	auto device = deviceResources->GetDevice();

	HRESULT hr;
	for (int i = 0; i < CFrameBufferCount; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_COMPUTE, IID_PPV_ARGS(commandAllocators[i].ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			throw std::runtime_error("Unable to create command allocators");
		}
	}

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_COMPUTE, commandAllocators[0].Get(), NULL, IID_PPV_ARGS(commandList.ReleaseAndGetAddressOf()));

	for (int i = 0; i < CFrameBufferCount; i++)
	{
		hr = device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fences[i].ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			throw std::runtime_error("Unable to create fences.");
		}
		fenceValues[i] = 0;
		fenceEvent[i] = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	}

	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	computeRootSignatureID = CreateComputeRootSignature();
	computeRootSignature = GContext->ResourceManager->GetRootSignature(computeRootSignatureID);
}

ID3D12CommandAllocator* ComputeContext::GetAllocator(uint32 index)
{
	return commandAllocators[index].Get();
}

ID3D12GraphicsCommandList* ComputeContext::GetDefaultCommandList()
{
	return commandList.Get();
}

ID3D12Device* ComputeContext::GetDevice()
{
	return deviceResources->GetDevice();
}

void ComputeContext::SubmitCommands(ID3D12GraphicsCommandList* commandList)
{
	auto commandQueue = deviceResources->GetCommandQueue();
	commandList->Close();
	ID3D12CommandList* ppCommandLists[] = { commandList };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);;
	fenceValues[backBufferIndex]++;
	auto hr = commandQueue->Signal(fences[backBufferIndex].Get(), fenceValues[backBufferIndex]);
}

void ComputeContext::WaitForFrame()
{
	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	WaitForFrame(backBufferIndex);
}

void ComputeContext::CleanUp()
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

void ComputeContext::ResetAllocator(ID3D12CommandAllocator* allocator)
{
	auto hr = allocator->Reset();
	if (FAILED(hr))
	{
	}
}

void ComputeContext::ResetCommandList(ID3D12GraphicsCommandList* commandList, ID3D12CommandAllocator* allocator)
{
	auto hr = commandList->Reset(allocator, nullptr);
	if (FAILED(hr))
	{
	}
}

void ComputeContext::CreateAllocator(D3D12_COMMAND_LIST_TYPE type, ID3D12CommandAllocator** allocator)
{
	auto device = deviceResources->GetDevice();
	device->CreateCommandAllocator(type, IID_PPV_ARGS(allocator));
}

uint64& ComputeContext::Fence(int index)
{
	return fenceValues[index];
}

ID3D12RootSignature* ComputeContext::GetComputeRootSignature()
{
	return computeRootSignature;
}

void ComputeContext::WaitForFrame(uint32 index)
{
	auto commandQueue = deviceResources->GetCommandQueue();
	backBufferIndex = index;
	if (fences[index]->GetCompletedValue() < fenceValues[index])
	{
		auto hr = fences[index]->SetEventOnCompletion(fenceValues[index], fenceEvent[index]);
		if (SUCCEEDED(hr))
		{
			fenceValues[index]++;
		}
		if (WaitForSingleObjectEx(fenceEvent[index], 100, FALSE) == WAIT_TIMEOUT)
		{
			WaitForGPU(index);
		}
	}


}

void ComputeContext::WaitForGPU(uint32 backBufferIndex)
{
	auto commandQueue = deviceResources->GetCommandQueue();

	// Signal command queue to update fence to given fence value. 
	if (SUCCEEDED(commandQueue->Signal(fences[backBufferIndex].Get(), fenceValues[backBufferIndex])))
	{
		auto hr = fences[backBufferIndex]->SetEventOnCompletion(fenceValues[backBufferIndex], fenceEvent[backBufferIndex]);
		if (SUCCEEDED(hr))
		{
			WaitForSingleObjectEx(fenceEvent[backBufferIndex], INFINITE, FALSE);
			fenceValues[backBufferIndex]++;
		}
	}
}