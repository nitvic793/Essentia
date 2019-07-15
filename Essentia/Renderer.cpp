#include "Renderer.h"
#include "Window.h"
#include "DeviceResources.h"
#include <wrl.h>

using namespace Microsoft::WRL;

void Renderer::Initialize()
{
	window = std::unique_ptr<Window>(new Window());
	deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());
	renderTargetManager = std::unique_ptr<RenderTargetManager>(new RenderTargetManager());

	window->Initialize(GetModuleHandle(0), 1920, 1080);
	deviceResources->Initialize(1920, 1080, window.get());
	renderTargetManager->Initialize(deviceResources->GetDevice());
	
	auto swapChain = deviceResources->GetSwapChain();
	frameBufferIndex = swapChain->GetCurrentBackBufferIndex();
	renderTargets.resize(CFrameBufferCount);

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		auto hr = swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargetBuffers[i].ReleaseAndGetAddressOf()));
		renderTargets[i] = renderTargetManager->CreateRenderTarget(renderTargetBuffers[i].Get());
	}

	InitializeCommandList();
}

void Renderer::Clear()
{
}

void Renderer::Render()
{
}

void Renderer::Present()
{
}

Window* Renderer::GetWindow()
{
	return window.get();
}

void Renderer::CleanUp()
{
	CloseHandle(fenceEvent);
}

void Renderer::InitializeCommandList()
{
	HRESULT hr;
	auto device = deviceResources->GetDevice();
	for (int i = 0; i < CFrameBufferCount; i++)
	{
		hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(commandAllocators[i].ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			throw std::runtime_error("Unable to create command allocators");
		}
	}

	hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), NULL, IID_PPV_ARGS(&commandList));

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
}
