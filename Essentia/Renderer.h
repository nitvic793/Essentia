#pragma once

#include <map>
#include <memory>

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
#include "Timer.h"
#include "BaseComponents.h"
#include "RenderBucket.h"
#include "FrameContext.h"
#include "RenderStage.h"
#include "MainPassRenderStage.h"
#include "Utility.h"
#include "PostProcess.h"

struct TransitionDesc
{
	ResourceID				Resource;
	D3D12_RESOURCE_STATES	From;
	D3D12_RESOURCE_STATES	To;
};

struct TransitionResourceDesc
{
	ID3D12Resource*			Resource;
	D3D12_RESOURCE_STATES	From;
	D3D12_RESOURCE_STATES	To;
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
	void	SetRenderTargets(RenderTargetID* renderTargets, int rtCount, DepthStencilID* depthStencilId, bool singleHandleToRTsDescriptorRange = false);

	ID3D12GraphicsCommandList*	GetDefaultCommandList();
	ID3D12Device*				GetDevice();
	MeshManager*				GetMeshManager();
	RenderTargetID				GetCurrentRenderTarget() const;
	TextureID					GetCurrentRenderTargetTexture() const;
	TextureID					GetCurrentHDRRenderTargetTexture() const;
	TextureID					GetPreviousHDRRenderTargetTexture() const;
	DepthStencilID				GetCurrentDepthStencil() const;
	TextureID					GetCurrentDepthStencilTexture() const;
	ID3D12RootSignature*		GetDefaultRootSignature() const;
	DXGI_FORMAT					GetRenderTargetFormat() const;
	DXGI_FORMAT					GetDepthStencilFormat() const;
	DXGI_FORMAT					GetHDRRenderTargetFormat() const;
	const GPUHeapOffsets&		GetHeapOffsets() const;
	FrameManager*				GetFrameManager() const;
	const D3D12_VIEWPORT&		GetViewport() const;
	const D3D12_RECT&			GetScissorRect() const;
	ScreenSize					GetScreenSize() const;
	void						DrawScreenQuad(ID3D12GraphicsCommandList* commandList);
	void						SetConstantBufferView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, const ConstantBufferView& view);
	void						SetShaderResourceView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID texture);
	void						SetShaderResourceViews(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID* textures, uint32 textureCount);
	void						TransitionBarrier(ID3D12GraphicsCommandList* commandList, ResourceID resourceId, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
	void						TransitionBarrier(ID3D12GraphicsCommandList* commandList, const TransitionDesc* transitions, uint32 count);
	void						TransitionBarrier(ID3D12GraphicsCommandList* commandList, const TransitionResourceDesc* transitions, uint32 count);
	void						TransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to);
	void						SetTargetSize(ID3D12GraphicsCommandList* commandList, ScreenSize screenSize);
	ID3D12Resource*				GetCurrentRenderTargetResource();
	void						SetDefaultRenderTarget();
	RenderTargetID				GetDefaultRenderTarget();
	RenderTargetID				GetDefaultHDRRenderTarget();
	void						SetVSync(bool enabled);
	void						Draw(ID3D12GraphicsCommandList* commandList, const RenderBucket& bucket);
	void						Draw(ID3D12GraphicsCommandList* commandList, DrawableModelComponent* drawableModels, uint32 count);
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
	uint32			prevBackBufferIndex = CFrameBufferCount - 1;
	RootSignatureID mainRootSignatureID;
	PipelineStateID	defaultPSO;
	DepthStencilID	depthStencilId;
	D3D12_VIEWPORT	viewport;
	D3D12_RECT		scissorRect;
	DXGI_FORMAT		renderTargetFormat;
	DXGI_FORMAT		hdrRenderTargetFormat;
	DXGI_FORMAT		depthFormat;
	ID3D12Device*	device;
	RenderBucket	renderBucket;
	ResourceID		depthBufferResourceId;

	//Temp -> will move to FrameManager
	PerObjectConstantBuffer perObject;
	LightBuffer				lightBuffer;
	ConstantBufferView		lightBufferView;
	ConstantBufferView		perObjectView;
	GPUHeapID				texID;
	GPUHeapOffsets			offsets;
	Material				material;
	bool					vsync = false;

	TextureID			irradianceTexture;
	TextureID			brdfLutTexture;
	TextureID			prefilterTexture;

	ModelManager							modelManager;
	std::map<RenderStageType, Vector<ScopedPtr<IRenderStage>>> renderStages;
	Vector<ScopedPtr<IPostProcessStage>>	postProcessStages;
	DescriptorHeap							imguiHeap;
	ScopedPtr<Window>						window;
	ScopedPtr<DeviceResources>				deviceResources;
	ScopedPtr<ResourceManager>				resourceManager;
	ScopedPtr<RenderTargetManager>			renderTargetManager;
	ScopedPtr<MeshManager>					meshManager;
	ScopedPtr<ShaderResourceManager>		shaderResourceManager;
	ScopedPtr<FrameManager>					frameManager;

	std::vector<RenderTargetID>				renderTargets;
	RenderTargetID							hdrRenderTargets[CFrameBufferCount];
	ResourceID								hdrRenderTargetResources[CFrameBufferCount];
	TextureID								hdrRenderTargetTextures[CFrameBufferCount];

	Microsoft::WRL::ComPtr<ID3D12Resource>	renderTargetBuffers[CFrameBufferCount];
	TextureID								renderTargetTextures[CFrameBufferCount];
	TextureID								depthStencilTexture;

	ScopedPtr<CommandContext>				commandContext;
};

template<typename T>
inline T* Renderer::Allocate()
{
	void* buffer = Mem::Alloc(sizeof(T));
	return new(buffer) T();
}
