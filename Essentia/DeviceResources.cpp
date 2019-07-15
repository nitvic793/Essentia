#include "DeviceResources.h"

DeviceResources::DeviceResources()
{
}

void DeviceResources::CreateDevice()
{
	auto hr = CreateDXGIFactory1(IID_PPV_ARGS(dxgiFactory.ReleaseAndGetAddressOf()));

	IDXGIAdapter1* adapter;
	int adapterIndex = 0;
	bool adapterFound = false;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			// we dont want a software device
			adapterIndex++;
			continue;
		}

		//direct3d 12 (feature level 11 or higher)
		hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1, _uuidof(ID3D12Device), nullptr);
		if (SUCCEEDED(hr))
		{
			adapterFound = true;
			break;
		}

		adapterIndex++;
	}

	if (!adapterFound)
	{
		throw std::runtime_error("Adapter not found");
	}

	hr = D3D12CreateDevice(
		adapter,
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(device.ReleaseAndGetAddressOf())
	);
}

void DeviceResources::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	auto hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()));
}

void DeviceResources::CreateSwapChain(int width, int height, Window* window)
{
	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = width;
	backBufferDesc.Height = height;
	backBufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_SAMPLE_DESC sampleDesc = {}; 
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = CFrameBufferCount; // number of buffers we have
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // this says the pipeline will render to this swap chain
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // dxgi will discard the buffer (data) after we call present
	swapChainDesc.OutputWindow = window->GetWindowHandle();
	swapChainDesc.SampleDesc = sampleDesc; // multi-sampling description
	swapChainDesc.Windowed = !window->IsFullscreen(); // set to true, then if in fullscreen must call SetFullScreenState with true for full screen to get uncapped fps

	dxgiFactory->CreateSwapChain(
		commandQueue.Get(),
		&swapChainDesc,
		(IDXGISwapChain**)swapChain.ReleaseAndGetAddressOf()
	);

	//swapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
}

DeviceResources::~DeviceResources()
{
}

void DeviceResources::Initialize(int width, int height, Window* window)
{
	auto hr = CoInitializeEx(nullptr, COINITBASE_MULTITHREADED); 
	ID3D12Debug1* debugInterface;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
	debugInterface->EnableDebugLayer();

	CreateDevice();
	CreateCommandQueue();
	CreateSwapChain(width, height, window);
}

void DeviceResources::Cleanup()
{
}

ID3D12Device* DeviceResources::GetDevice()
{
	return device.Get();
}

IDXGISwapChain3* DeviceResources::GetSwapChain()
{
	return swapChain.Get();
}

ID3D12CommandQueue* DeviceResources::GetCommandQueue()
{
	return commandQueue.Get();
}
