#pragma once

#include "Declarations.h"
#include "Window.h"
#include "DeviceResources.h"
#include "RenderTargetManager.h"
#include "ResourceManager.h"
#include "Camera.h"
#include "CommandContext.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include "DXUtility.h"
#include "ShaderResourceManager.h"
#include <memory>
#include "Timer.h"

struct FrameContext
{
	Camera* Camera;
	Timer* timer;
};

enum RootParameterSlot {
	RootSigCBVertex0 = 0,
	RootSigCBPixel0,
	RootSigSRVPixel1,
	RootSigCBAll1,
	RootSigCBAll2
};

class Renderer
{
public:
	void	Initialize();
	void	Clear();
	void	Render(const FrameContext& frameContext);
	void	Present();
	Window* GetWindow();
	void	CleanUp();
	void	EndInitialization();

	ID3D12GraphicsCommandList*	GetDefaultCommandList();
	ID3D12Device*				GetDevice();
	MeshManager*				GetMeshManager();
private:
	void InitializeCommandContext();
	void CreateRootSignatures();
	void CreatePSOs();
	void CreateDepthStencil();
	void WaitForPreviousFrame();

	int32			width;
	int32			height;
	uint32			backBufferIndex;
	RootSignatureID mainRootSignatureID;
	PipelineStateID	defaultPSO;
	DepthStencilID	depthStencilId;
	D3D12_VIEWPORT	viewport;
	D3D12_RECT		scissorRect;
	DXGI_FORMAT		renderTargetFormat;
	DXGI_FORMAT		depthFormat;
	ID3D12Device*	device;

	//Temp -> will move to FrameManager
	GPUConstantBuffer cbuffer;
	PerObjectConstantBuffer perObject;
	LightBuffer			lightBuffer;
	ConstantBufferView	lightBufferView;
	ConstantBufferView  perObjectView;
	GPUHeapID			texID;
	GPUHeapOffsets		offsets;

	std::unique_ptr<Window>					window;
	std::unique_ptr<DeviceResources>		deviceResources;
	std::unique_ptr<ResourceManager>		resourceManager;
	std::unique_ptr<RenderTargetManager>	renderTargetManager;
	std::unique_ptr<MeshManager>			meshManager;
	std::unique_ptr<ShaderResourceManager>	shaderResourceManager;
	std::unique_ptr<FrameManager>			frameManager;

	std::vector<RenderTargetID>				renderTargets;
	Microsoft::WRL::ComPtr<ID3D12Resource>	renderTargetBuffers[CFrameBufferCount];

	std::unique_ptr<CommandContext>			commandContext;
};

