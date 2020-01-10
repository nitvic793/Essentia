#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "Window.h"
#include "DXUtility.h"

class DeviceResources
{
public:
	~DeviceResources();
	void Initialize(Window* window, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	void Cleanup();

	ID3D12Device*		GetDevice();
	IDXGISwapChain3*	GetSwapChain();
	ID3D12CommandQueue* GetCommandQueue();
	ID3D12CommandQueue* GetComputeQueue();
private:
	DeviceResources();
	Microsoft::WRL::ComPtr<ID3D12Device>		device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue>	computeQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3>		swapChain;
	Microsoft::WRL::ComPtr<IDXGIFactory4>		dxgiFactory;
	Window* window;

	void CreateDevice();
	void CreateCommandQueue();
	void CreateSwapChain(Window* window, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

	friend class Renderer;
};

