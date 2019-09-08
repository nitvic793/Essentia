#include "Renderer.h"
#include "Window.h"
#include "DeviceResources.h"
#include <wrl.h>
#include "d3dx12.h"
#include <thread>
#include "ShaderManager.h"
#include "InputLayout.h"
#include <array>
#include "pix3.h"
#include "Engine.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx12.h"
#include "Entity.h"
#include "ImguiRenderStage.h"
#include "SkyBoxRenderStage.h"
#include "OutlineRenderStage.h"
#include "PostProcessDepthOfFieldStage.h"

using namespace Microsoft::WRL;
using namespace DirectX;
MeshView mesh;

void Renderer::Initialize()
{
	renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
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
	gpuMemory = MakeScopedArgs<GraphicsMemory>(device);

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

	CreateDepthStencil();
	InitializeCommandContext();
	CreateRootSignatures();
	CreatePSOs();

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

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		auto hr = swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargetBuffers[i].ReleaseAndGetAddressOf()));
		renderTargets[i] = renderTargetManager->CreateRenderTargetView(renderTargetBuffers[i].Get(), renderTargetFormat);
	}

	for (size_t i = 0; i < CFrameBufferCount; ++i)
	{
		renderTargetTextures[i] = shaderResourceManager->CreateTexture(renderTargetBuffers[i].Get());
	}

	for (size_t i = 0; i < eRenderTypeCount; ++i)
	{
		Vector<ScopedPtr<IRenderStage>> stages(16);
		renderStages.insert(std::pair<RenderStageType, Vector<ScopedPtr<IRenderStage>>>((RenderStageType)i, std::move(stages)));
	}

	renderStages[eRenderStageMain].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<MainPassRenderStage>()));
	renderStages[eRenderStageMain].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<SkyBoxRenderStage>()));


	renderStages[eRenderStageGUI].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<OutlineRenderStage>()));
	renderStages[eRenderStageGUI].Push(ScopedPtr<IRenderStage>((IRenderStage*)Mem::Alloc<ImguiRenderStage>()));

	postProcessStages.Reserve(32);
	postProcessStages.Push(ScopedPtr<IPostProcessStage>((IPostProcessStage*)Mem::Alloc<PostProcessDepthOfFieldStage>()));

	auto dir = XMVector3Normalize(XMVectorSet(1, -1, 1, 0));
	XMStoreFloat3(&lightBuffer.DirLight.Direction, dir);
	lightBuffer.DirLight.Color = XMFLOAT3(0.9f, 0.9f, 0.9f);
	lightBuffer.PointLight.Color = XMFLOAT3(0.9f, 0.1f, 0.1f);
	lightBuffer.PointLight.Position = XMFLOAT3(2.9f, 0.1f, 0.1f);
	lightBuffer.PointLight.Range = 5.f;
	lightBuffer.PointLight.Intensity = 2.f;

	for (auto& stageSegement : renderStages)
	{
		for (auto& stage : stageSegement.second)
			stage->Initialize();
	}

	for (auto& stage : postProcessStages)
	{
		stage->Initialize();
	}

	window->RegisterOnResizeCallback([&]()
		{
			if (!commandContext.Get()) return;
			for (int i = 0; i < CFrameBufferCount; ++i)
			{
				commandContext->WaitForFrame(i);
			}
			for (int i = 0; i < CFrameBufferCount; ++i)
			{
				renderTargetBuffers[i].ReleaseAndGetAddressOf();
			}

			resourceManager->Release(depthBufferResourceId);

			deviceResources->GetSwapChain()->ResizeBuffers(CFrameBufferCount, width, height, renderTargetFormat, 0);
			for (int i = 0; i < CFrameBufferCount; ++i)
			{
				auto hr = swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargetBuffers[i].ReleaseAndGetAddressOf()));
				renderTargetManager->ReCreateRenderTargetView(renderTargets[i], renderTargetBuffers[i].Get(), renderTargetFormat);
			}

		});
}

void Renderer::Clear()
{
	WaitForPreviousFrame();
	auto imageIndex = backBufferIndex;
	auto commandAllocator = commandContext->GetAllocator(backBufferIndex);
	auto commandList = commandContext->GetDefaultCommandList();

	commandContext->ResetAllocator(commandAllocator);
	commandContext->ResetCommandList(commandList, commandAllocator);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetBuffers[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto rtId = renderTargets[backBufferIndex];
	auto rtv = renderTargetManager->GetRTVHandle(rtId);
	auto dsv = renderTargetManager->GetDSVHandle(depthStencilId);

	const float clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
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

void Renderer::Render(const FrameContext& frameContext)
{
	auto camera = frameContext.Camera;
	perObject.View = camera->GetViewTransposed();
	perObject.Projection = camera->GetProjectionTransposed();

	UpdateLightBuffer();
	lightBuffer.CameraPosition = camera->Position;

	auto worlds = frameContext.WorldMatrices;
	auto drawables = frameContext.Drawables;
	auto drawCount = frameContext.DrawableCount;
	auto imageIndex = backBufferIndex;
	uint32 drawableModelCount;
	auto drawableModels = frameContext.EntityManager->GetComponents<DrawableModelComponent>(drawableModelCount);
	auto entities = frameContext.EntityManager->GetEntities<DrawableModelComponent>(drawableModelCount);
	std::vector<XMFLOAT4X4> modelWorlds;
	modelWorlds.reserve(drawableModelCount);
	frameContext.EntityManager->GetTransposedWorldMatrices(entities, drawableModelCount, modelWorlds);

	//Copy world matrix to constant buffer
	for (size_t i = 0; i < drawCount; ++i)
	{
		perObject.World = worlds[i];
		shaderResourceManager->CopyToCB(imageIndex, { &perObject, sizeof(perObject) }, drawables[i].CBView.Offset);
		renderBucket.Insert(drawables[i], 0);
	}

	for (size_t i = 0; i < drawableModelCount; ++i)
	{
		perObject.World = modelWorlds[i];
		shaderResourceManager->CopyToCB(imageIndex, { &perObject, sizeof(perObject) }, drawableModels[i].CBView.Offset);
	}

	shaderResourceManager->CopyToCB(imageIndex, { &lightBuffer, sizeof(LightBuffer) }, lightBufferView.Offset);
	//offsets = shaderResourceManager->CopyDescriptorsToGPUHeap(imageIndex, frameManager.get()); //TO DO: Copy fixed resources to heap first and only copy dynamic resources per frame

	auto rtId = renderTargets[backBufferIndex];
	auto commandList = commandContext->GetDefaultCommandList();

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	this->SetRenderTargets(&rtId, 1, &depthStencilId);

	commandList->SetGraphicsRootSignature(resourceManager->GetRootSignature(mainRootSignatureID));

	std::array<ID3D12DescriptorHeap*, 1> heaps = { frameManager->GetGPUDescriptorHeap(imageIndex) };
	commandList->SetDescriptorHeaps((UINT)heaps.size(), heaps.data());

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

	commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetGraphicsRootDescriptorTable(RootSigCBPixel0, frameManager->GetHandle(imageIndex, offsets.ConstantBufferOffset + lightBufferView.Index));
	commandList->SetGraphicsRootDescriptorTable(RootSigIBL, frameManager->GetHandle(imageIndex, offsets.TexturesOffset + irradianceTexture));

	for (auto pipeline : renderBucket.Pipelines)
	{
		auto psoBucket = pipeline.second;
		auto pso = psoBucket.PipelineStateObject;
		commandList->SetPipelineState(pso);
		for (auto mat : psoBucket.Instances)
		{
			auto material = mat.second.Material;
			commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frameManager->GetHandle(imageIndex, offsets.MaterialsOffset + material.StartIndex));
			for (auto meshes : mat.second.Instances)
			{
				auto mesh = meshes.second.Mesh;
				commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView);
				commandList->IASetIndexBuffer(&mesh.IndexBufferView);
				for (uint32 cbIndex : meshes.second.CbIndices)
				{
					commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frameManager->GetHandle(imageIndex, offsets.ConstantBufferOffset + cbIndex));
					for (auto& m : mesh.MeshEntries)
					{
						commandList->DrawIndexedInstanced(m.NumIndices, 1, m.BaseIndex, m.BaseVertex, 0);
					}
				}
			}
		}
	}

	for (size_t i = 0; i < drawableModelCount; ++i)
	{
		auto& model = modelManager.GetModel(drawableModels[i].Model);
		commandList->SetGraphicsRootDescriptorTable(RootSigCBVertex0, frameManager->GetHandle(imageIndex, offsets.ConstantBufferOffset + drawableModels[i].CBView.Index));
		auto meshHandle = model.Mesh;
		auto mesh = meshManager->GetMeshView(meshHandle);
		int matIndex = 0;
		commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView);
		commandList->IASetIndexBuffer(&mesh.IndexBufferView);
		for (auto& m : mesh.MeshEntries)
		{
			auto materialHandle = model.Materials[matIndex];
			auto material = shaderResourceManager->GetMaterial(materialHandle);
			commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frameManager->GetHandle(imageIndex, offsets.MaterialsOffset + material.StartIndex));
			commandList->DrawIndexedInstanced(m.NumIndices, 1, m.BaseIndex, m.BaseVertex, 0);
			matIndex++;
		}
	}

	for (auto& stage : renderStages[eRenderStageMain]) //Main Render Pass
	{
		stage->Render(backBufferIndex, frameContext);
	}

	TransitionBarrier(commandList, renderTargetBuffers[backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	for (auto& postProcessStage : postProcessStages)
	{
		postProcessStage->RenderPostProcess(backBufferIndex, renderTargetTextures[backBufferIndex]);
	}

	TransitionBarrier(commandList, renderTargetBuffers[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

	SetDefaultRenderTarget();

	for (auto& renderStage : renderStages[eRenderStageGUI]) //GUI Render pass
	{
		renderStage->Render(imageIndex, frameContext);
	}

	PIXEndEvent(commandList);
}

void Renderer::Present()
{
	auto commandList = commandContext->GetDefaultCommandList();
	auto swapChain = deviceResources->GetSwapChain();

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetBuffers[backBufferIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
	commandContext->SubmitCommands(commandList);
	auto hr = swapChain->Present(0, 0);
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
	Default::DefaultDiffuse = shaderResourceManager->CreateTexture("../../Assets/Textures/floor_albedo.png");
	Default::DefaultNormals = shaderResourceManager->CreateTexture("../../Assets/Textures/floor_normals.png");
	Default::DefaultRoughness = shaderResourceManager->CreateTexture("../../Assets/Textures/defaultRoughness.png");
	Default::DefaultMetalness = shaderResourceManager->CreateTexture("../../Assets/Textures/defaultMetal.png");

	auto meshId = meshManager->CreateMesh("../../Assets/Models/sphere.obj", mesh);
	perObjectView = shaderResourceManager->CreateCBV(sizeof(PerObjectConstantBuffer));
	lightBufferView = shaderResourceManager->CreateCBV(sizeof(LightBuffer));

	TextureID textures[MaterialTextureCount];
	textures[DiffuseID] = shaderResourceManager->CreateTexture("../../Assets/Textures/floor_albedo.png");
	textures[NormalsID] = shaderResourceManager->CreateTexture("../../Assets/Textures/floor_normals.png");
	textures[RoughnessID] = shaderResourceManager->CreateTexture("../../Assets/Textures/floor_roughness.png");
	textures[MetalnessID] = shaderResourceManager->CreateTexture("../../Assets/Textures/floor_metal.png");
	auto matId = shaderResourceManager->CreateMaterial(textures, 4, defaultPSO, material);
	modelManager.CreateModel("../../Assets/Models/Sponza.fbx");

	irradianceTexture = shaderResourceManager->CreateTexture("../../Assets/IBL/envDiffuseHDR.dds", DDS, false);
	brdfLutTexture = shaderResourceManager->CreateTexture("../../Assets/IBL/envBrdf.dds", DDS, false);
	prefilterTexture = shaderResourceManager->CreateTexture("../../Assets/IBL/envSpecularHDR.dds", DDS, false);

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		offsets = shaderResourceManager->CopyDescriptorsToGPUHeap(i, frameManager.get());
	}

	auto commandList = commandContext->GetDefaultCommandList();
	commandContext->SubmitCommands(commandList);
}

void Renderer::DrawMesh(const MeshView& meshView)
{
	auto commandList = commandContext->GetDefaultCommandList();
	commandList->IASetVertexBuffers(0, 1, &meshView.VertexBufferView);
	commandList->IASetIndexBuffer(&meshView.IndexBufferView);
	commandList->DrawIndexedInstanced(meshView.IndexCount, 1, 0, 0, 0);
}

void Renderer::DrawMesh(MeshHandle mesh)
{
	auto commandList = commandContext->GetDefaultCommandList();
	auto& meshView = meshManager->GetMeshView(mesh);
	DrawMesh(meshView);
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
	commandList->OMSetRenderTargets(rtCount, handles.GetData(), singleHandleToRTsDescriptorRange, dsvHandle);
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

DepthStencilID Renderer::GetCurrentDepthStencil() const
{
	return depthStencilId;
}

ID3D12RootSignature* Renderer::GetDefaultRootSignature() const
{
	return resourceManager->GetRootSignature(mainRootSignatureID);
}

DXGI_FORMAT Renderer::GetRenderTargetFormat() const
{
	return renderTargetFormat;
}

DXGI_FORMAT Renderer::GetDepthStencilFormat() const
{
	return depthFormat;
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

void Renderer::SetShaderResourceView(ID3D12GraphicsCommandList* commandList, RootParameterSlot slot, TextureID texture)
{
	commandList->SetGraphicsRootDescriptorTable(slot, frameManager->GetHandle(backBufferIndex, offsets.TexturesOffset + texture));
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

void Renderer::TransitionBarrier(ID3D12GraphicsCommandList* commandList, ID3D12Resource* resource, D3D12_RESOURCE_STATES from, D3D12_RESOURCE_STATES to)
{
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(resource, from, to));
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

void Renderer::InitializeCommandContext()
{
	commandContext = MakeScoped<CommandContext>();
	commandContext->Initialize(deviceResources.get());
}

void Renderer::CreateRootSignatures()
{
	CD3DX12_DESCRIPTOR_RANGE range[6];
	//view dependent CBV
	range[RootSigCBVertex0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//light dependent CBV
	range[RootSigCBPixel0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//G-Buffer inputs
	range[RootSigSRVPixel1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 16, 0);
	//per frame CBV
	range[RootSigCBAll1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	//per bone 
	range[RootSigCBAll2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);
	//IBL Textures
	range[RootSigIBL].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 4, 16);

	CD3DX12_ROOT_PARAMETER rootParameters[6];
	rootParameters[RootSigCBVertex0].InitAsDescriptorTable(1, &range[RootSigCBVertex0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[RootSigCBPixel0].InitAsDescriptorTable(1, &range[RootSigCBPixel0], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootSigSRVPixel1].InitAsDescriptorTable(1, &range[RootSigSRVPixel1], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootSigCBAll1].InitAsDescriptorTable(1, &range[RootSigCBAll1], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[RootSigCBAll2].InitAsDescriptorTable(1, &range[RootSigCBAll2], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[RootSigIBL].InitAsDescriptorTable(1, &range[RootSigIBL], D3D12_SHADER_VISIBILITY_PIXEL);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(6, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS);

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[2];
	StaticSamplers[0].Init(0, D3D12_FILTER_ANISOTROPIC, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, D3D12_TEXTURE_ADDRESS_MODE_WRAP, 0.f, 4);
	StaticSamplers[1].Init(1, D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		D3D12_TEXTURE_ADDRESS_MODE_BORDER,
		0.f, 16u, D3D12_COMPARISON_FUNC_LESS_EQUAL);
	descRootSignature.NumStaticSamplers = 2;
	descRootSignature.pStaticSamplers = StaticSamplers;

	mainRootSignatureID = resourceManager->CreateRootSignature(descRootSignature);
}

void Renderer::CreatePSOs()
{
	auto vertexShaderBytecode = ShaderManager::LoadShader(L"DefaultVS.cso");
	auto pixelShaderBytecode = ShaderManager::LoadShader(L"DefaultPS.cso");

	DXGI_SAMPLE_DESC sampleDesc = {};
	sampleDesc.Count = 1;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
	psoDesc.InputLayout.pInputElementDescs = InputLayout::DefaultLayout;
	psoDesc.InputLayout.NumElements = _countof(InputLayout::DefaultLayout);
	psoDesc.pRootSignature = resourceManager->GetRootSignature(mainRootSignatureID);
	psoDesc.VS = vertexShaderBytecode;
	psoDesc.PS = pixelShaderBytecode;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc = sampleDesc;
	psoDesc.SampleMask = 0xffffffff;
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.NumRenderTargets = 1;
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DSVFormat = depthFormat;

	defaultPSO = resourceManager->CreatePSO(psoDesc);
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
}

void Renderer::WaitForPreviousFrame()
{
	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	commandContext->WaitForFrame(backBufferIndex);
}

void Renderer::UpdateLightBuffer()
{
	auto ec = EngineContext::Context;
	uint32 dirLightCount = 0;
	uint32 pointLightCount = 0;
	auto dirLights = ec->EntityManager->GetComponents<DirectionalLightComponent>(dirLightCount);
	auto pointLights = ec->EntityManager->GetComponents<PointLightComponent>(pointLightCount);
	lightBuffer.DirLight = dirLights[0].GetLight();

	auto entities = ec->EntityManager->GetEntities<PointLightComponent>(pointLightCount);
	lightBuffer.PointLight = pointLights[0].GetLight();
	lightBuffer.PointLight.Position = *ec->EntityManager->GetTransform(entities[0]).Position;
}
