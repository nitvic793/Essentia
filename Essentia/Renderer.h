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
#include "ComputeContext.h"
#include "Mesh.h"
#include "ConstantBuffer.h"
#include "DXUtility.h"
#include "ShaderResourceManager.h"
#include "Timer.h"
#include "BaseComponents.h"
#include "RenderComponents.h"
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
	void	DrawMesh(ID3D12GraphicsCommandList* commandList, const MeshView& meshView);
	void	DrawAnimatedMesh(ID3D12GraphicsCommandList* commandList, const MeshView& meshView);
	void    DrawMesh(ID3D12GraphicsCommandList* commandList, MeshHandle mesh);
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
	ID3D12RootSignature*		GetDefaultComputeRootSignature() const;
	DXGI_FORMAT					GetRenderTargetFormat() const;
	DXGI_FORMAT					GetDepthStencilFormat() const;
	DXGI_FORMAT					GetHDRRenderTargetFormat() const;
	const GPUHeapOffsets&		GetHeapOffsets() const;
	FrameManager*				GetFrameManager() const;
	const D3D12_VIEWPORT&		GetViewport() const;
	const D3D12_RECT&			GetScissorRect() const;
	ScreenSize					GetScreenSize() const;
	uint32						GetCurrentBackbufferIndex() const;
	D3D12_GPU_DESCRIPTOR_HANDLE	GetTextureGPUHandle(TextureID textureId) const;
	ComputeContext*				GetComputeContext() const;
	void						DrawScreenQuad(ID3D12GraphicsCommandList* commandList);
	void						SetConstantBufferView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, const ConstantBufferView& view);
	void						SetComputeConstantBufferView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, const ConstantBufferView& view);
	void						SetShaderResourceView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID texture);
	void						SetComputeShaderResourceView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID texture);
	void						SetShaderResourceViewMaterial(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, MaterialHandle material);
	void						SetShaderResourceViews(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID* textures, uint32 textureCount);
	void						SetComputeShaderResourceViews(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID* textures, uint32 textureCount);
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
	void						DrawAnimated(ID3D12GraphicsCommandList* commandList);
	void						Draw(ID3D12GraphicsCommandList* commandList, const RenderBucket& bucket, const Camera* camera);
	void						Draw(ID3D12GraphicsCommandList* commandList, DrawableModelComponent* drawableModels, uint32 count, const Camera* camera);
	void						SetPipelineState(ID3D12GraphicsCommandList* commandList, PipelineStateID pipelineStateId);					
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
	PerFrameConstantBuffer	perFrame;
	LightBuffer				lightBuffer;
	ConstantBufferView		lightBufferView;
	ConstantBufferView		perObjectView;
	ConstantBufferView		perFrameView;
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
	::DescriptorHeap						imguiHeap;
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
	ScopedPtr<ComputeContext>				computeContext;
};

template<typename T>
inline T* Renderer::Allocate()
{
	void* buffer = Mem::Alloc(sizeof(T));
	return new(buffer) T();
}
