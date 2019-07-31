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

	device->SetStablePowerState(FALSE);

	{
		Microsoft::WRL::ComPtr<ID3D12InfoQueue> d3dInfoQueue;
		if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&d3dInfoQueue))))
		{
			d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

			D3D12_MESSAGE_ID blockedIds[] = { D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,
			  D3D12_MESSAGE_ID_CLEARDEPTHSTENCILVIEW_MISMATCHINGCLEARVALUE, D3D12_MESSAGE_ID_COPY_DESCRIPTORS_INVALID_RANGES };
			D3D12_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.pIDList = blockedIds;
			filter.DenyList.NumIDs = 3;
			d3dInfoQueue->AddRetrievalFilterEntries(&filter);
			d3dInfoQueue->AddStorageFilterEntries(&filter);
		}
	}
}

void DeviceResources::CreateCommandQueue()
{
	D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	auto hr = device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(commandQueue.GetAddressOf()));
}

void DeviceResources::CreateSwapChain(Window* window, DXGI_FORMAT format)
{
	auto windowSize = window->GetWindowSize();
	DXGI_MODE_DESC backBufferDesc = {};
	backBufferDesc.Width = windowSize.Width;
	backBufferDesc.Height = windowSize.Height;
	backBufferDesc.Format = format;
	DXGI_SAMPLE_DESC sampleDesc = {}; 
	sampleDesc.Count = 1;

	DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
	swapChainDesc.BufferCount = CFrameBufferCount; 
	swapChainDesc.BufferDesc = backBufferDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; 
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.OutputWindow = window->GetWindowHandle();
	swapChainDesc.SampleDesc = sampleDesc; 
	swapChainDesc.Windowed = !window->IsFullscreen(); 
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; 

	dxgiFactory->CreateSwapChain(
		commandQueue.Get(),
		&swapChainDesc,
		(IDXGISwapChain**)swapChain.ReleaseAndGetAddressOf()
	);
}

DeviceResources::~DeviceResources()
{
}

void DeviceResources::Initialize(Window* window, DXGI_FORMAT format)
{
	ID3D12Debug1* debugInterface;
	D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
	debugInterface->EnableDebugLayer();

	CreateDevice();
	CreateCommandQueue();
	CreateSwapChain(window, format);
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
