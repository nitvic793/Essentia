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
#include "BaseComponents.h"
#include "RenderBucket.h"
#include "FrameContext.h"
#include "RenderStage.h"
#include "MainPassRenderStage.h"
#include <GraphicsMemory.h>

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
	void	DrawMesh(const MeshView& meshView);
	void    DrawMesh(MeshHandle mesh);

	ID3D12GraphicsCommandList*	GetDefaultCommandList();
	ID3D12Device*				GetDevice();
	MeshManager*				GetMeshManager();
	RenderTargetID				GetCurrentRenderTarget() const;
	DepthStencilID				GetCurrentDepthStencil() const;
	ID3D12RootSignature*		GetDefaultRootSignature() const;
	DXGI_FORMAT					GetRenderTargetFormat() const;
	DXGI_FORMAT					GetDepthStencilFormat() const;
	const GPUHeapOffsets&		GetHeapOffsets() const;
	FrameManager*				GetFrameManager() const;
private:
	void InitializeCommandContext();
	void CreateRootSignatures();
	void CreatePSOs();
	void CreateDepthStencil();
	void WaitForPreviousFrame();
	void UpdateLightBuffer();

	template<typename T>
	T* Allocate();

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
	RenderBucket	renderBucket;

	//Temp -> will move to FrameManager
	GPUConstantBuffer cbuffer;
	PerObjectConstantBuffer perObject;
	LightBuffer			lightBuffer;
	ConstantBufferView	lightBufferView;
	ConstantBufferView  perObjectView;
	GPUHeapID			texID;
	GPUHeapOffsets		offsets;
	Material			material;

	ModelManager							modelManager;
	std::vector<ScopedPtr<IRenderStage>>	renderStages;
	DescriptorHeap							imguiHeap;
	std::unique_ptr<Window>						window;
	ScopedPtr<DeviceResources>				deviceResources;
	ScopedPtr<ResourceManager>				resourceManager;
	ScopedPtr<RenderTargetManager>			renderTargetManager;
	ScopedPtr<MeshManager>					meshManager;
	ScopedPtr<ShaderResourceManager>		shaderResourceManager;
	ScopedPtr<FrameManager>					frameManager;
	ScopedPtr<DirectX::GraphicsMemory>		gpuMemory;
	std::vector<RenderTargetID>				renderTargets;
	Microsoft::WRL::ComPtr<ID3D12Resource>	renderTargetBuffers[CFrameBufferCount];

	std::unique_ptr<CommandContext>				commandContext;
};

template<typename T>
inline T* Renderer::Allocate()
{
	void* buffer = Mem::Alloc(sizeof(T));
	return new(buffer) T();
}
