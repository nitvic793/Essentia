#include <array>
#include "pix3.h"
#include <wrl.h>
#include "d3dx12.h"
#include <thread>
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"

#include "Renderer.h"
#include "Window.h"
#include "DeviceResources.h"
#include "ShaderManager.h" 
#include "InputLayout.h"
#include "Engine.h"
#include "Entity.h"

#include "DepthOnlyStage.h"
#include "ShadowRenderStage.h"
#include "ImguiRenderStage.h"
#include "SkyBoxRenderStage.h"
#include "OutlineRenderStage.h"
#include "PostProcessTemporalAA.h"
#include "VelocityBufferStage.h"
#include "PostProcessMotionBlur.h"
#include "PostProcessDepthOfFieldStage.h"
#include "PostProcessToneMap.h"
#include "DebugDrawStage.h"
#include "ScreenSpaceAOStage.h"
#include "VolumetricLightStage.h"
#include "ApplyVolumetricFog.h"
#include "VoxelizationStage.h"
#include "GenerateMipsStage.h"
#include "VoxelRadiancePostProcess.h"

#include "PipelineStates.h"
#include "SceneResources.h"

using namespace Microsoft::WRL;
using namespace DirectX;

void Renderer::Initialize()
{
	renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	hdrRenderTargetFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
	width = 1920;
	height = 1080;
	depthFormat = DXGI_FORMAT_D32_FLOAT;

	window = ScopedPtr<Window>(Allocate<Window>());
	deviceResources = ScopedPtr<DeviceResources>(Allocate<DeviceResources>());
	renderTargetManager = ScopedPtr<RenderTargetManager>(Allocate<RenderTargetManager>());
	resourceManager = ScopedPtr<ResourceManager>(Allocate<ResourceManager>());
	meshManager = ScopedPtr<MeshManager>(Allocate<MeshManager>());
	shaderResourceManager = ScopedPtr<ShaderResourceManager>(Allocate<ShaderResourceManager>());
	frameManager = ScopedPtr<FrameManager>(Allocate<FrameManager>());

	window->Initialize(GetModuleHandle(0), width, height, "Essentia", "Essentia", true);
	deviceResources->Initialize(window.get(), renderTargetFormat);

	device = deviceResources->GetDevice();
	renderTargetManager->Initialize(device);
	resourceManager->Initialize(device);

	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	renderTargets.resize(CFrameBufferCount);

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = width;
	scissorRect.bottom = height;

	InitializeCommandContext();
	CreateRootSignatures();


	meshManager->Initialize(commandContext.get());
	shaderResourceManager->Initialize(resourceManager.get(), deviceResources.get());
	frameManager->Initialize(device);
	modelManager.Initialize(shaderResourceManager.get());

	auto ec = EngineContext::Context;
	ec->MeshManager = meshManager.get();
	ec->ResourceManager = resourceManager.get();
	ec->ShaderResourceManager = shaderResourceManager.get();
	ec->CommandContext = commandContext.get();
	ec->DeviceResources = deviceResources.get();
	ec->RenderTargetManager = renderTargetManager.get();
	ec->ModelManager = &modelManager;
	ec->FrameManager = frameManager.get();

	CreateDepthStencil();
	computeContext = MakeScoped<ComputeContext>();
	computeContext->Initialize(deviceResources.get());
	computeContext->GetDefaultCommandList()->Close();

	GPipelineStates.Initialize();
	GPostProcess.Intitialize();
	GSceneResources.Initalize();
	CreatePSOs();

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		auto hr = swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargetBuffers[i].ReleaseAndGetAddressOf()));
		renderTargets[i] = renderTargetManager->CreateRenderTargetView(renderTargetBuffers[i].Get(), renderTargetFormat);
	}

	for (size_t i = 0; i < CFrameBufferCount; ++i)
	{
		renderTargetTextures[i] = shaderResourceManager->CreateTexture(renderTargetBuffers[i].Get());
	}

	for (size_t i = 0; i < CFrameBufferCount; ++i)
	{
		TextureCreateProperties props = { (uint32)width, (uint32)height, DXGI_FORMAT_R32G32B32A32_FLOAT };
		ResourceID resourceId;
		auto textureId = shaderResourceManager->CreateTexture2D(props, &resourceId);
		hdrRenderTargets[i] = renderTargetManager->CreateRenderTargetView(resourceManager->GetResource(resourceId), hdrRenderTargetFormat);
		hdrRenderTargetResources[i] = resourceId;
		hdrRenderTargetTextures[i] = textureId;
	}

	for (size_t i = 0; i < eRenderTypeCount; ++i)
	{
		Vector<ScopedPtr<IRenderStage>> stages(16);
		renderStages.insert(std::pair<RenderStageType, Vector<ScopedPtr<IRenderStage>>>((RenderStageType)i, std::move(stages)));
	}

	renderStages[eRenderStagePreMain].Push(ScopedPtr<IRenderStage>(Allocate<DepthOnlyStage>()));
	renderStages[eRenderStagePreMain].Push(ScopedPtr<IRenderStage>(Allocate<ScreenSpaceAOStage>()));
	renderStages[eRenderStagePreMain].Push(ScopedPtr<IRenderStage>(Allocate<ShadowRenderStage>()));
	renderStages[eRenderStagePreMain].Push(ScopedPtr<IRenderStage>(Allocate<VoxelizationStage>()));
	renderStages[eRenderStagePreMain].Push(ScopedPtr<IRenderStage>(Allocate<VolumetricLightStage>()));


	renderStages[eRenderStageCompute].Push(ScopedPtr<IRenderStage>(Allocate<VoxelRadiancePostProcess>()));
	renderStages[eRenderStageCompute].Push(ScopedPtr<IRenderStage>(Allocate<VoxelMipGenStage>()));

	renderStages[eRenderStageMain].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<MainPassRenderStage>()));
	renderStages[eRenderStageMain].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<SkyBoxRenderStage>()));
	

	renderStages[eRenderStageGUI].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<DebugDrawStage>()));
	renderStages[eRenderStageGUI].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<OutlineRenderStage>()));
	renderStages[eRenderStageGUI].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<ImguiRenderStage>()));

	postProcessStages.Reserve(32);
	postProcessStages.Push(ScopedPtr<IPostProcessStage>((IPostProcessStage*)Mem::Alloc<VelocityBufferStage>()));
	postProcessStages.Push(ScopedPtr<IPostProcessStage>((IPostProcessStage*)Mem::Alloc<PostProcessTemporalAA>()));
	postProcessStages.Push(ScopedPtr<IPostProcessStage>((IPostProcessStage*)Mem::Alloc<ApplyVolumetricFog>()));

	postProcessStages.Push(ScopedPtr<IPostProcessStage>((IPostProcessStage*)Mem::Alloc<PostProcessMotionBlur>()));
	postProcessStages.Push(ScopedPtr<IPostProcessStage>((IPostProcessStage*)Mem::Alloc<PostProcessDepthOfFieldStage>()));
	postProcessStages.Push(ScopedPtr<IPostProcessStage>((IPostProcessStage*)Mem::Alloc<PostProcessToneMap>()));

	for (auto& stageSegment : renderStages)
	{
		for (auto& stage : stageSegment.second)
			stage->Initialize();
	}

	for (auto& stage : postProcessStages)
	{
		stage->Initialize();
	}

	window->RegisterOnResizeCallback([&]()
		{
			auto swapChain = deviceResources->GetSwapChain();
			if (!commandContext.Get()) return;
			auto bindex = swapChain->GetCurrentBackBufferIndex();
			commandContext->WaitForGPU(bindex);
			for (int i = 0; i < CFrameBufferCount; ++i)
			{
				renderTargetBuffers[i].Reset();
				commandContext->FenceValue(i) = commandContext->FenceValue(bindex);
			}

			swapChain->ResizeBuffers(CFrameBufferCount, width, height, renderTargetFormat, 0);

			for (int i = 0; i < CFrameBufferCount; ++i)
			{
				auto hr = swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargetBuffers[i]));
				renderTargetManager->ReCreateRenderTargetView(renderTargets[i], renderTargetBuffers[i].Get(), renderTargetFormat);
			}

			if (!window->IsFullscreen())
				swapChain->SetFullscreenState(FALSE, nullptr);
		});
}

void Renderer::Clear()
{
	WaitForPreviousFrame();
	auto imageIndex = backBufferIndex;

	//Reset Compute Context
	auto computeAllocator = computeContext->GetAllocator(backBufferIndex);
	auto computeCList = computeContext->GetDefaultCommandList();
	computeContext->ResetAllocator(computeAllocator);
	computeContext->ResetCommandList(computeCList, computeAllocator);

	auto commandAllocator = commandContext->GetAllocator(backBufferIndex);
	auto commandList = commandContext->GetDefaultCommandList();

	commandContext->ResetAllocator(commandAllocator);
	commandContext->ResetCommandList(commandList, commandAllocator);

	TransitionResourceDesc frameEndTransitions[] = {
		{resourceManager->GetResource(hdrRenderTargetResources[backBufferIndex]), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET},
		{renderTargetBuffers[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET}
	};

	TransitionBarrier(commandList, frameEndTransitions, _countof(frameEndTransitions));

	auto rtId = renderTargets[backBufferIndex];
	auto hdrTarget = hdrRenderTargets[backBufferIndex];
	auto rtv = renderTargetManager->GetRTVHandle(rtId);
	auto hdrRtv = renderTargetManager->GetRTVHandle(hdrTarget);
	auto dsv = renderTargetManager->GetDSVHandle(depthStencilId);

	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	commandList->ClearRenderTargetView(hdrRtv, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
	frameManager->Reset(imageIndex);
	renderBucket.Clear();

	for (auto& renderStage : renderStages)
	{
		for (auto& stage : renderStage.second)
		{
			stage->Clear();
		}
	}
}

/***
TODO:
1. Use ambient IBL if Voxelization is disabled - Shader switching system
2. Frustum & Occlusion Culling
3. Fix Global Illumination
*/
void Renderer::Render(const FrameContext& frameContext)
{
	auto commandList = commandContext->GetDefaultCommandList();
	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Begin Frame");

	auto renderStageMap = GRenderStageManager.GetRenderStageMap();
	uint32 camCount;
	const auto& camera = frameContext.EntityManager->GetComponents<CameraComponent>(camCount)[0].CameraInstance;
	auto imageIndex = backBufferIndex;

	perObject.View = camera.GetViewTransposed();
	perObject.Projection = camera.GetProjectionTransposed();

	UpdateLightBuffer();
	lightBuffer.CameraPosition = camera.Position;

	auto& worlds = frameContext.WorldMatrices;
	auto drawables = frameContext.Drawables;
	auto drawCount = frameContext.DrawableCount;

	auto& modelWorlds = frameContext.ModelWorldMatrices;
	auto drawableModelCount = frameContext.DrawableModelCount;
	auto drawableModels = frameContext.DrawableModels;

	auto projection = XMLoadFloat4x4(&camera.Projection);
	auto view = XMLoadFloat4x4(&camera.View);

	//Copy world matrix to constant buffer
	for (size_t i = 0; i < drawCount; ++i)
	{
		auto world = XMMatrixTranspose(XMLoadFloat4x4(&worlds[i]));
		XMStoreFloat4x4(&drawables[i].WorldViewProjection, XMMatrixTranspose(world * view * projection));

		perObject.PrevWorldViewProjection = drawables[i].PrevWorldViewProjection;
		perObject.World = worlds[i];
		perObject.WorldViewProjection = drawables[i].WorldViewProjection;
		shaderResourceManager->CopyToCB(imageIndex, { &perObject, sizeof(perObject) }, drawables[i].CBView);

		auto bounding = meshManager->GetBoundingBox(drawables[i].Mesh);
		bounding.Transform(bounding, world);
		if (camera.Frustum.Contains(bounding)) // TODO: Make Frustum Culling System 
			renderBucket.Insert(drawables[i], 0);
		drawables[i].PrevWorldViewProjection = drawables[i].WorldViewProjection;
	}

	for (size_t i = 0; i < drawableModelCount; ++i)
	{
		auto world = XMMatrixTranspose(XMLoadFloat4x4(&modelWorlds[i]));
		XMStoreFloat4x4(&drawableModels[i].WorldViewProjection, XMMatrixTranspose(world * view * projection));

		perObject.World = modelWorlds[i];
		perObject.PrevWorldViewProjection = drawableModels[i].PrevWorldViewProjection;
		perObject.WorldViewProjection = drawableModels[i].WorldViewProjection;
		shaderResourceManager->CopyToCB(imageIndex, { &perObject, sizeof(perObject) }, drawableModels[i].CBView);
		drawableModels[i].PrevWorldViewProjection = drawableModels[i].WorldViewProjection;
	}

	lightBuffer.NearZ = camera.NearZ;
	lightBuffer.FarZ = camera.FarZ;
	shaderResourceManager->CopyToCB(imageIndex, { &lightBuffer, sizeof(LightBuffer) }, lightBufferView);

	offsets = shaderResourceManager->CopyDescriptorsToGPUHeap(imageIndex, frameManager.get()); //TO DO: Copy fixed resources to heap first and only copy dynamic resources per frame

	XMMATRIX T(
		0.5f, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f, 0.5f, 0.0f, 1.0f);
	XMMATRIX viewProjTex = XMMatrixMultiply(XMMatrixMultiply(view, projection), T);
	//Update per frame data
	XMStoreFloat4x4(&GSceneResources.FrameData.ViewProjectionTex, XMMatrixTranspose(viewProjTex));
	auto voxelData = CreateVoxelParams(camera, CVoxelSize);
	if (XMVector3Equal(XMLoadFloat3(&voxelData.VoxelGridCenter), (XMLoadFloat3(&GSceneResources.FrameData.VoxelData.VoxelGridCenter))))
	{
		//renderStageMap["VoxelizationStage"]->Enabled = false;
		//renderStageMap["VoxelMipGenStage"]->Enabled = false;
		//renderStageMap["VoxelRadiancePostProcess"]->Enabled = false;
	}
	else
	{
		//renderStageMap["VoxelizationStage"]->Enabled = true;
		//renderStageMap["VoxelMipGenStage"]->Enabled = true;
		//renderStageMap["VoxelRadiancePostProcess"]->Enabled = true;
	}

	GSceneResources.FrameData.VoxelData = voxelData;
	shaderResourceManager->CopyToCB(imageIndex, { &GSceneResources.FrameData, sizeof(GSceneResources.FrameData) }, perFrameView);

	auto rtId = renderTargets[backBufferIndex];
	auto hdrRtId = hdrRenderTargets[backBufferIndex];
	std::array<ID3D12DescriptorHeap*, 1> heaps = { frameManager->GetGPUDescriptorHeap(imageIndex) };

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	this->SetRenderTargets(&hdrRtId, 1, &depthStencilId);
	commandList->SetGraphicsRootSignature(resourceManager->GetRootSignature(mainRootSignatureID));
	commandList->SetDescriptorHeaps((UINT)heaps.size(), heaps.data());
	commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"PrePass Stage");

	for (auto& stage : renderStages[eRenderStagePreMain])
	{
		PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Sub Render Stage");
		if (stage->Enabled)
		{
			stage->Render(backBufferIndex, frameContext);
		}

		PIXEndEvent(commandList);
	}

	PIXEndEvent(commandList);

	commandContext->SubmitCommands(commandList);
	commandContext->ResetCommandList(commandList, commandContext->GetAllocator(backBufferIndex));

	/***COMPUTE**/
	auto computeCList = computeContext->GetDefaultCommandList();
	auto gfxFence = commandContext->GetFence();
	auto gfxFenceValue = commandContext->FenceValue(backBufferIndex);

	deviceResources->computeQueue->Wait(gfxFence, gfxFenceValue);

	computeCList->SetComputeRootSignature(computeContext->GetComputeRootSignature());
	computeCList->SetDescriptorHeaps((UINT)heaps.size(), heaps.data());

	PIXBeginEvent(computeCList, PIX_COLOR_DEFAULT, L"Pre Compute Stage");

	for (auto& stage : renderStages[eRenderStageCompute])
	{
		PIXBeginEvent(computeCList, PIX_COLOR_DEFAULT, L"Sub Compute Stage");
		if (stage->Enabled)
		{
			stage->Render(backBufferIndex, frameContext);
		}

		PIXEndEvent(computeCList);
	}

	PIXEndEvent(computeCList);

	computeContext->SubmitCommands(computeCList);

	/***END COMPUTE**/

	// Wait for compute work to be over
	auto computeFence = computeContext->GetFence();
	auto computeFenceValue = computeContext->FenceValue(backBufferIndex);
	deviceResources->commandQueue->Wait(computeFence, computeFenceValue);

	if (renderStageMap["VoxelizationStage"]->Enabled)
		TransitionBarrier(commandList, GSceneResources.VoxelGridResource, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	this->SetRenderTargets(&hdrRtId, 1, &depthStencilId);
	commandList->SetGraphicsRootSignature(resourceManager->GetRootSignature(mainRootSignatureID));
	commandList->SetDescriptorHeaps((UINT)heaps.size(), heaps.data());
	commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Main Render Stage");

	// Set constant buffer for lights in Pixel Shader
	commandList->SetGraphicsRootDescriptorTable(RootSigCBPixel0, frameManager->GetHandle(imageIndex, offsets.ConstantBufferOffset + lightBufferView.Index));
	// Set textures for IBL. Here we are setting 3 textures, irradiance texture is at index 0 while 
	commandList->SetGraphicsRootDescriptorTable(RootSigIBL, frameManager->GetHandle(imageIndex, offsets.TexturesOffset + irradianceTexture));
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);

	TextureID textures[] = { GSceneResources.ShadowDepthTarget.Texture , GSceneResources.AmbientOcclusion.Texture, GSceneResources.VoxelGridSRV };
	SetShaderResourceViews(commandList, RootSigSRVPixel2, textures, _countof(textures));
	SetConstantBufferView(commandList, RootSigCBAll1, perFrameView);
	SetConstantBufferView(commandList, RootSigCBAll2, GSceneResources.ShadowCBV);
	this->SetRenderTargets(&hdrRtId, 1, &depthStencilId);

	Draw(commandList, renderBucket, &camera);
	Draw(commandList, drawableModels, drawableModelCount, &camera);

	for (auto& stage : renderStages[eRenderStageMain]) //Main Render Pass
	{
		PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Sub Render Stage");
		if (stage->Enabled)
		{
			stage->Render(backBufferIndex, frameContext);
		}
		PIXEndEvent(commandList);
	}

	TransitionResourceDesc mainHdrTransitions[] = {
		{shaderResourceManager->GetResource(hdrRenderTargetTextures[backBufferIndex]), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE },
		{resourceManager->GetResource(depthBufferResourceId), D3D12_RESOURCE_STATE_DEPTH_WRITE, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE}
	};
	TransitionBarrier(commandList, mainHdrTransitions, _countof(mainHdrTransitions));
	PIXEndEvent(commandList);

	PIXEndEvent(commandList);

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Post Process");
	GPostProcess.GenerateLowResTextures();
	auto inputTexture = hdrRenderTargetTextures[backBufferIndex];
	for (auto& postProcessStage : postProcessStages)
	{
		if (postProcessStage->Enabled)
		{
			inputTexture = postProcessStage->RenderPostProcess(backBufferIndex, inputTexture, frameContext);
		}
	}

	TransitionDesc postPPTransitions[] = {
		{ depthBufferResourceId, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_DEPTH_WRITE },
		{ GSceneResources.PreviousFrame.Resource, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET }
	};

	TransitionBarrier(commandList, postPPTransitions, _countof(postPPTransitions));
	PIXEndEvent(commandList);

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render PrevFrame Target");
	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.HDRQuadPSO));
	SetRenderTargets(&GSceneResources.PreviousFrame.RenderTarget, 1, nullptr);
	SetShaderResourceView(commandList, RootSigSRVPixel1, hdrRenderTargetTextures[backBufferIndex]);
	DrawScreenQuad(commandList);

	TransitionBarrier(commandList, GSceneResources.PreviousFrame.Resource, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	PIXEndEvent(commandList);

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render Scene Main Target");
	commandList->SetPipelineState(resourceManager->GetPSO(GPipelineStates.QuadPSO));
	SetDefaultRenderTarget();
	SetShaderResourceView(commandList, RootSigSRVPixel1, inputTexture);
	DrawScreenQuad(commandList);
	PIXEndEvent(commandList);

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"GUI");
	for (auto& renderStage : renderStages[eRenderStageGUI]) //GUI Render pass
	{
		if (renderStage->Enabled)
		{
			renderStage->Render(imageIndex, frameContext);
		}
	}
	PIXEndEvent(commandList);

	PIXEndEvent(commandList); // End Frame
}

void Renderer::Present()
{
	auto commandList = commandContext->GetDefaultCommandList();
	auto swapChain = deviceResources->GetSwapChain();

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetBuffers[backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	commandContext->SubmitCommands(commandList);
	auto hr = swapChain->Present(vsync ? 1 : 0, 0);
	if (FAILED(hr))
	{
		hr = device->GetDeviceRemovedReason();
	}
}

Window* Renderer::GetWindow()
{
	return window.get();
}

void Renderer::CleanUp()
{
	commandContext->CleanUp();
	for (auto& renderStage : renderStages)
	{
		for (auto& stage : renderStage.second)
			stage->CleanUp();
	}
}

void Renderer::EndInitialization()
{
	//Load Default Textures
	Default::DefaultDiffuse = shaderResourceManager->CreateTexture("Assets/Textures/floor_albedo.png");
	Default::DefaultNormals = shaderResourceManager->CreateTexture("Assets/Textures/floor_normals.png");
	Default::DefaultRoughness = shaderResourceManager->CreateTexture("Assets/Textures/defaultRoughness.png");
	Default::DefaultMetalness = shaderResourceManager->CreateTexture("Assets/Textures/defaultMetal.png");

	TextureID textures[MaterialTextureCount];
	textures[DiffuseID] = shaderResourceManager->CreateTexture("Assets/Textures/floor_albedo.png");
	textures[NormalsID] = shaderResourceManager->CreateTexture("Assets/Textures/floor_normals.png");
	textures[RoughnessID] = shaderResourceManager->CreateTexture("Assets/Textures/floor_roughness.png");
	textures[MetalnessID] = shaderResourceManager->CreateTexture("Assets/Textures/floor_metal.png");
	auto matId = shaderResourceManager->CreateMaterial(textures, 4, defaultPSO, material);

	irradianceTexture = shaderResourceManager->CreateTexture("Assets/IBL/envDiffuseHDR.dds", DDS, false);
	brdfLutTexture = shaderResourceManager->CreateTexture("Assets/IBL/envBrdf.dds", DDS, false);
	prefilterTexture = shaderResourceManager->CreateTexture("Assets/IBL/envSpecularHDR.dds", DDS, false);

	perObjectView = shaderResourceManager->CreateCBV(sizeof(PerObjectConstantBuffer));
	lightBufferView = shaderResourceManager->CreateCBV(sizeof(LightBuffer));
	perFrameView = shaderResourceManager->CreateCBV(sizeof(PerFrameConstantBuffer));
	GSceneResources.LightBufferCBV = lightBufferView;
	GSceneResources.FrameDataCBV = perFrameView;

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		offsets = shaderResourceManager->CopyDescriptorsToGPUHeap(i, frameManager.get());
	}

	auto commandList = commandContext->GetDefaultCommandList();
	commandContext->SubmitCommands(commandList);
}

void Renderer::DrawMesh(ID3D12GraphicsCommandList* commandList, const MeshView& meshView)
{
	commandList->IASetVertexBuffers(0, 1, &meshView.VertexBufferView);
	commandList->IASetIndexBuffer(&meshView.IndexBufferView);
	for (auto& m : meshView.MeshEntries)
	{
		commandList->DrawIndexedInstanced(m.NumIndices, 1, m.BaseIndex, m.BaseVertex, 0);
	}
}

void Renderer::DrawMesh(ID3D12GraphicsCommandList* commandList, MeshHandle mesh)
{
	auto& meshView = meshManager->GetMeshView(mesh);
	DrawMesh(commandList, meshView);
}

void Renderer::SetRenderTargets(RenderTargetID* renderTargets, int rtCount, DepthStencilID* depthStencilId, bool singleHandleToRTsDescriptorRange)
{
	Vector<D3D12_CPU_DESCRIPTOR_HANDLE> handles(rtCount);
	for (size_t i = 0; i < rtCount; ++i)
	{
		handles.Push(renderTargetManager->GetRTVHandle(renderTargets[i]));
	}

	auto* dsvHandle = depthStencilId == nullptr ? (D3D12_CPU_DESCRIPTOR_HANDLE*)nullptr : &renderTargetManager->GetDSVHandle(*depthStencilId);
	auto commandList = GetDefaultCommandList();

	D3D12_CPU_DESCRIPTOR_HANDLE* handleList = rtCount == 0 ? nullptr : handles.GetData();
	commandList->OMSetRenderTargets(rtCount, handleList, singleHandleToRTsDescriptorRange, dsvHandle);
}

ID3D12GraphicsCommandList* Renderer::GetDefaultCommandList()
{
	return commandContext->GetDefaultCommandList();
}

ID3D12Device* Renderer::GetDevice()
{
	return device;
}

MeshManager* Renderer::GetMeshManager()
{
	return meshManager.get();
}

RenderTargetID Renderer::GetCurrentRenderTarget() const
{
	return renderTargets[backBufferIndex];
}

TextureID Renderer::GetCurrentRenderTargetTexture() const
{
	return renderTargetTextures[backBufferIndex];
}

TextureID Renderer::GetCurrentHDRRenderTargetTexture() const
{
	return hdrRenderTargetTextures[backBufferIndex];
}

TextureID Renderer::GetPreviousHDRRenderTargetTexture() const
{
	return hdrRenderTargetTextures[prevBackBufferIndex];
}

DepthStencilID Renderer::GetCurrentDepthStencil() const
{
	return depthStencilId;
}

TextureID Renderer::GetCurrentDepthStencilTexture() const
{
	return depthStencilTexture;
}

ID3D12RootSignature* Renderer::GetDefaultRootSignature() const
{
	return resourceManager->GetRootSignature(mainRootSignatureID);
}

ID3D12RootSignature* Renderer::GetDefaultComputeRootSignature() const
{
	return computeContext->GetComputeRootSignature();
}

DXGI_FORMAT Renderer::GetRenderTargetFormat() const
{
	return renderTargetFormat;
}

DXGI_FORMAT Renderer::GetDepthStencilFormat() const
{
	return depthFormat;
}

DXGI_FORMAT Renderer::GetHDRRenderTargetFormat() const
{
	return hdrRenderTargetFormat;
}

const GPUHeapOffsets& Renderer::GetHeapOffsets() const
{
	return offsets;
}

FrameManager* Renderer::GetFrameManager() const
{
	return frameManager.get();
}

const D3D12_VIEWPORT& Renderer::GetViewport() const
{
	return viewport;
}

const D3D12_RECT& Renderer::GetScissorRect() const
{
	return scissorRect;
}

ScreenSize Renderer::GetScreenSize() const
{
	return ScreenSize{ width, height };
}

uint32 Renderer::GetCurrentBackbufferIndex() const
{
	return deviceResources->GetSwapChain()->GetCurrentBackBufferIndex();
}

D3D12_GPU_DESCRIPTOR_HANDLE Renderer::GetTextureGPUHandle(TextureID textureId) const
{
	return frameManager->GetHandle(backBufferIndex, offsets.TexturesOffset + textureId);
}

ComputeContext* Renderer::GetComputeContext() const
{
	return computeContext.Get();
}

void Renderer::DrawScreenQuad(ID3D12GraphicsCommandList* commandList)
{
	D3D12_INDEX_BUFFER_VIEW ibv;
	ibv.Format = DXGI_FORMAT_R32_UINT;
	ibv.BufferLocation = 0;
	ibv.SizeInBytes = 0;

	D3D12_VERTEX_BUFFER_VIEW vbv;
	vbv.BufferLocation = 0;
	vbv.SizeInBytes = 0;
	vbv.StrideInBytes = 0;
	commandList->IASetVertexBuffers(0, 0, &vbv);
	commandList->IASetIndexBuffer(&ibv);
	commandList->DrawInstanced(4, 1, 0, 0);
}

void Renderer::SetConstantBufferView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, const ConstantBufferView& view)
{
	commandList->SetGraphicsRootDescriptorTable(slot, frameManager->GetHandle(backBufferIndex, offsets.ConstantBufferOffset + view.Index));
}

void Renderer::SetComputeConstantBufferView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, const ConstantBufferView& view)
{
	commandList->SetComputeRootDescriptorTable(slot, frameManager->GetHandle(backBufferIndex, offsets.ConstantBufferOffset + view.Index));
}

void Renderer::SetShaderResourceView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID texture)
{
	commandList->SetGraphicsRootDescriptorTable(slot, frameManager->GetHandle(backBufferIndex, offsets.TexturesOffset + texture));
}

void Renderer::SetComputeShaderResourceView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID texture)
{
	commandList->SetComputeRootDescriptorTable(slot, frameManager->GetHandle(backBufferIndex, offsets.TexturesOffset + texture));
}

void Renderer::SetShaderResourceViewMaterial(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, MaterialHandle material)
{
	auto mat = shaderResourceManager->GetMaterial(material);
	auto handle = frameManager->GetHandle(backBufferIndex, offsets.MaterialsOffset + mat.StartIndex);
	commandList->SetGraphicsRootDescriptorTable(slot, handle);
}

void Renderer::SetShaderResourceViews(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID* textures, uint32 textureCount)
{
	auto handle = shaderResourceManager->AllocateTextures(textures, textureCount, backBufferIndex, frameManager.get());
	commandList->SetGraphicsRootDescriptorTable(slot, handle);
}

void Renderer::SetComputeShaderResourceViews(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID* textures, uint32 textureCount)
{
	auto handle = shaderResourceManager->AllocateTextures(textures, textureCount, backBufferIndex, frameManager.get());
	commandList->SetComputeRootDescriptorTable(slot, handle);
}

void Renderer::TransitionBarrier(ID3D12GraphicsCommandList* commandList, ResourceID resourceId, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
{
	auto resource = resourceManager->GetResource(resourceId);
	TransitionBarrier(commandList, resource, from, to);
}

void Renderer::TransitionBarrier(ID3D12GraphicsCommandList* commandList, const TransitionDesc* transitions, uint32 count)
{
	Vector<CD3DX12_RESOURCE_BARRIER> barriers;
	barriers.Reserve(count);

	for (size_t i = 0; i < count; ++i)
	{
		auto resource = resourceManager->GetResource(transitions[i].Resource);
		barriers.Push(CD3DX12_RESOURCE_BARRIER::Transition(resource, transitions[i].From, transitions[i].To));
	}

	commandList->ResourceBarrier(count, barriers.GetData());
}

void Renderer::TransitionBarrier(ID3D12GraphicsCommandList* commandList, const TransitionResourceDesc* transitions, uint32 count)
{
	Vector<CD3DX12_RESOURCE_BARRIER> barriers;
	barriers.Reserve(count);
	for (size_t i = 0; i < count; ++i)
	{
		barriers.Push(CD3DX12_RESOURCE_BARRIER::Transition(transitions[i].Resource, transitions[i].From, transitions[i].To));
	}

	commandList->ResourceBarrier(count, barriers.GetData());
}

void Renderer::TransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, from, to));
}

void Renderer::SetTargetSize(ID3D12GraphicsCommandList* commandList, ScreenSize screenSize)
{
	D3D12_VIEWPORT viewport = {};
	viewport.Width = (FLOAT)screenSize.Width;
	viewport.Height = (FLOAT)screenSize.Height;

	D3D12_RECT scissorRect = {};
	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = screenSize.Width;
	scissorRect.bottom = screenSize.Height;

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
}

ID3D12Resource* Renderer::GetCurrentRenderTargetResource()
{
	return renderTargetBuffers[backBufferIndex].Get();
}

void Renderer::SetDefaultRenderTarget()
{
	auto rtId = renderTargets[backBufferIndex];
	auto commandList = commandContext->GetDefaultCommandList();

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	this->SetRenderTargets(&rtId, 1, &depthStencilId);
}

RenderTargetID Renderer::GetDefaultRenderTarget()
{
	return renderTargets[backBufferIndex];
}

RenderTargetID Renderer::GetDefaultHDRRenderTarget()
{
	return hdrRenderTargets[backBufferIndex];
}

void Renderer::SetVSync(bool enabled)
{
	vsync = enabled;
}

void Renderer::Draw(ID3D12GraphicsCommandList* commandList, const RenderBucket& bucket, const Camera* camera)
{
	for (auto pipeline : bucket.Pipelines)
	{
		auto psoBucket = pipeline.second;
		auto pso = psoBucket.PipelineStateObject;
		// Set Pipeline. Defines the shaders and other pipeline states.
		commandList->SetPipelineState(pso);
		for (auto mat : psoBucket.Instances)
		{
			auto material = mat.second.Material;
			// Set material textures start index. Typically has 4 textures per PBR material.
			commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frameManager->GetHandle(backBufferIndex, offsets.MaterialsOffset + material.StartIndex));
			for (auto meshes : mat.second.Instances)
			{
				auto mesh = meshes.second.Mesh;
				commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView);
				commandList->IASetIndexBuffer(&mesh.IndexBufferView);
				for (uint32 cbIndex : meshes.second.CbIndices)
				{
					//Set constant buffer index for object being drawn in Vertex shader.
					commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frameManager->GetHandle(backBufferIndex, offsets.ConstantBufferOffset + cbIndex));
					DrawMesh(commandList, mesh);
				}
			}
		}
	}
}

void Renderer::Draw(ID3D12GraphicsCommandList* commandList, DrawableModelComponent* drawableModels, uint32 count, const Camera* camera)
{
	for (size_t i = 0; i < count; ++i)
	{
		auto& model = modelManager.GetModel(drawableModels[i].Model);
		commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frameManager->GetHandle(backBufferIndex, offsets.ConstantBufferOffset + drawableModels[i].CBView.Index));
		auto meshHandle = model.Mesh;
		auto mesh = meshManager->GetMeshView(meshHandle);
		int matIndex = 0;
		commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView);
		commandList->IASetIndexBuffer(&mesh.IndexBufferView);
		for (auto& m : mesh.MeshEntries)
		{
			auto materialHandle = model.Materials[matIndex]; //material maps to each mesh entry in model.
			auto material = shaderResourceManager->GetMaterial(materialHandle);
			commandList->SetPipelineState(resourceManager->GetPSO(material.PipelineID));
			commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frameManager->GetHandle(backBufferIndex, offsets.MaterialsOffset + material.StartIndex));
			commandList->DrawIndexedInstanced(m.NumIndices, 1, m.BaseIndex, m.BaseVertex, 0);
			matIndex++;
		}
	}
}

void Renderer::SetPipelineState(ID3D12GraphicsCommandList* commandList, PipelineStateID pipelineStateId)
{
	ID3D12PipelineState* pso = resourceManager->GetPSO(pipelineStateId);
	commandList->SetPipelineState(pso);
}

void Renderer::InitializeCommandContext()
{
	commandContext = MakeScoped<CommandContext>();
	commandContext->Initialize(deviceResources.get());
}

void Renderer::CreateRootSignatures()
{
	CD3DX12_DESCRIPTOR_RANGE range[RootSigParamCount];
	//view dependent CBV
	range[RootSigCBVertex0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//light dependent CBV
	range[RootSigCBPixel0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//G-Buffer inputs
	range[RootSigSRVPixel1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 0);
	//Extra Textures
	range[RootSigSRVPixel2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 8, 8);
	//per frame CBV
	range[RootSigCBAll1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	//per bone 
	range[RootSigCBAll2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	//IBL Textures
	range[RootSigIBL].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 16);
	// UAV textures
	range[RootSigUAV0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 4, 0);

	CD3DX12_ROOT_PARAMETER rootParameters[RootSigParamCount];
	rootParameters[RootSigCBVertex0].InitAsDescriptorTable(1, &range[RootSigCBVertex0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[RootSigCBPixel0].InitAsDescriptorTable(1, &range[RootSigCBPixel0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootSigSRVPixel1].InitAsDescriptorTable(1, &range[RootSigSRVPixel1], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootSigSRVPixel2].InitAsDescriptorTable(1, &range[RootSigSRVPixel2], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootSigCBAll1].InitAsDescriptorTable(1, &range[RootSigCBAll1], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[RootSigCBAll2].InitAsDescriptorTable(1, &range[RootSigCBAll2], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[RootSigIBL].InitAsDescriptorTable(1, &range[RootSigIBL], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootSigUAV0].InitAsDescriptorTable(1, &range[RootSigUAV0], D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(RootSigParamCount, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS);

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[4];
	//Base Sampler
	StaticSamplers[0].Init(0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.f, 4);
	//Shadow Sampler
	StaticSamplers[1].Init(1, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.f, 16u, D3D12_COMPARISON_FUNC_LESS_EQUAL);

	StaticSamplers[2].Init(2, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	StaticSamplers[3].Init(3, D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	descRootSignature.NumStaticSamplers = _countof(StaticSamplers);
	descRootSignature.pStaticSamplers = StaticSamplers;

	mainRootSignatureID = resourceManager->CreateRootSignature(descRootSignature);
}

void Renderer::CreatePSOs()
{
	defaultPSO = GPipelineStates.DefaultPSO;
}

void Renderer::CreateDepthStencil()
{
	auto clearVal = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.f, 0);
	depthBufferResourceId = resourceManager->CreateResource(
		CD3DX12_RESOURCE_DESC::Tex2D(depthFormat, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		&clearVal,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);

	auto depthBuffer = resourceManager->GetResource(depthBufferResourceId);
	depthStencilId = renderTargetManager->CreateDepthStencilView(depthBuffer, depthFormat);

	depthStencilTexture = shaderResourceManager->CreateTexture(depthBuffer, false, nullptr, DXGI_FORMAT_R32_FLOAT);
}

void Renderer::WaitForPreviousFrame()
{
	prevBackBufferIndex = backBufferIndex;
	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	commandContext->WaitForFrame();
	computeContext->WaitForFrame();
}

void Renderer::UpdateLightBuffer()
{
	auto ec = EngineContext::Context;
	uint32 dirLightCount = 0;
	uint32 pointLightCount = 0;
	uint32 spotLightCount = 0;
	auto dirLights = ec->EntityManager->GetComponents<DirectionalLightComponent>(dirLightCount);
	auto pointLights = ec->EntityManager->GetComponents<PointLightComponent>(pointLightCount);
	auto spotLights = ec->EntityManager->GetComponents<SpotLightComponent>(spotLightCount);
	lightBuffer.DirLightCount = dirLightCount;
	lightBuffer.DirLights[0] = dirLights[0].GetLight();

	auto entities = ec->EntityManager->GetEntities<PointLightComponent>(pointLightCount);
	lightBuffer.PointLightCount = pointLightCount;
	if (pointLightCount > 0)
	{
		for (uint32 i = 0; i < pointLightCount; ++i)
		{
			lightBuffer.PointLights[i] = pointLights[i].GetLight();
			lightBuffer.PointLights[i].Position = *ec->EntityManager->GetTransform(entities[i]).Position;
		}
	}

	entities = ec->EntityManager->GetEntities<SpotLightComponent>(spotLightCount);
	lightBuffer.SpotLightCount = spotLightCount;
	if (spotLightCount > 0)
	{
		for (uint32 i = 0; i < spotLightCount; ++i)
		{
			auto direction = (DirectX::XMFLOAT3) * ec->EntityManager->GetComponent<RotationComponent>(entities[i]);
			spotLights[i].Direction = direction;
			lightBuffer.SpotLights[i] = spotLights[i].GetLight();
			lightBuffer.SpotLights[i].Position = (DirectX::XMFLOAT3) * ec->EntityManager->GetComponent<PositionComponent>(entities[i]);
		}

	}
}
