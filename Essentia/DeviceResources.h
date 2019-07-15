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
	void Initialize(int width, int height, Window* window);
	void Cleanup();

	ID3D12Device*		GetDevice();
	IDXGISwapChain3*	GetSwapChain();
	ID3D12CommandQueue* GetCommandQueue();
private:
	DeviceResources();
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<IDXGIFactory4> dxgiFactory;

	void CreateDevice();
	void CreateCommandQueue();
	void CreateSwapChain(int width, int height, Window* window);

	friend class Renderer;
};

