#pragma once
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include "Window.h"
#include "DXUtility.h"

constexpr int CFrameBufferCount = 3;

class DeviceResources
{
public:
	~DeviceResources();
	void Initialize(Window* window, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);
	void Cleanup();

	ID3D12Device*		GetDevice();
	IDXGISwapChain3*	GetSwapChain();
	ID3D12CommandQueue* GetCommandQueue();
private:
	DeviceResources();
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;

	void CreateDevice();
	void CreateCommandQueue();
	void CreateSwapChain(Window* window, DXGI_FORMAT format = DXGI_FORMAT_R8G8B8A8_UNORM);

	friend class Renderer;
};

