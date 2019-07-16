#pragma once

#include "Declarations.h"
#include "Window.h"
#include "DeviceResources.h"
#include "RenderTargetManager.h"
#include "ResourceManager.h"

#include <memory>

class Renderer
{
public:
	void	Initialize();
	void	Clear();
	void	Render();
	void	Present();
	Window* GetWindow();
	void	CleanUp();
	void	EndInitialization();
private:
	void InitializeCommandList();
	void CreateRootSignatures();
	void CreatePSOs();
	void CreateDepthStencil();
	void WaitForPreviousFrame();

	int32			width;
	int32			height;
	uint32			backBufferIndex;
	RootSignatureID mainRootSignatureID;
	DepthStencilID	depthStencilId;
	D3D12_VIEWPORT	viewport;
	D3D12_RECT		scissorRect;

	std::unique_ptr<Window>					window;
	std::unique_ptr<DeviceResources>		deviceResources;
	std::unique_ptr<ResourceManager>		resourceManager;
	std::unique_ptr<RenderTargetManager>	renderTargetManager;

	std::vector<RenderTargetID>				renderTargets;
	Microsoft::WRL::ComPtr<ID3D12Resource>	renderTargetBuffers[CFrameBufferCount];

	Microsoft::WRL::ComPtr<ID3D12CommandAllocator>					commandAllocators[CFrameBufferCount];
	Microsoft::WRL::ComPtr<ID3D12Fence>								fences[CFrameBufferCount];
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList>				commandList;
	uint64															fenceValues[CFrameBufferCount];
	HANDLE															fenceEvent;
};

