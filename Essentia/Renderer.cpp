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

using namespace Microsoft::WRL;
using namespace DirectX;
MeshView mesh;

void Renderer::Initialize()
{
	renderTargetFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	width = 1920;
	height = 1080;
	depthFormat = DXGI_FORMAT_D32_FLOAT;

	window = std::unique_ptr<Window>(new Window());
	deviceResources = std::unique_ptr<DeviceResources>(new DeviceResources());
	renderTargetManager = std::unique_ptr<RenderTargetManager>(new RenderTargetManager());
	resourceManager = std::unique_ptr<ResourceManager>(new ResourceManager());
	meshManager = std::unique_ptr<MeshManager>(new MeshManager());
	shaderResourceManager = std::unique_ptr<ShaderResourceManager>(new ShaderResourceManager());
	frameManager = std::unique_ptr<FrameManager>(new FrameManager());

	window->Initialize(GetModuleHandle(0), width, height, "Essentia", "Essentia", true);
	deviceResources->Initialize(window.get(), renderTargetFormat);

	device = deviceResources->GetDevice();
	renderTargetManager->Initialize(device);
	resourceManager->Initialize(device);

	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	renderTargets.resize(CFrameBufferCount);

	for (int i = 0; i < CFrameBufferCount; ++i)
	{
		auto hr = swapChain->GetBuffer(i, IID_PPV_ARGS(renderTargetBuffers[i].ReleaseAndGetAddressOf()));
		renderTargets[i] = renderTargetManager->CreateRenderTargetView(renderTargetBuffers[i].Get(), renderTargetFormat);
	}

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

	imguiHeap.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1, true);
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(window->GetWindowHandle());
	ImGui_ImplDX12_Init(device, CFrameBufferCount,
		DXGI_FORMAT_R8G8B8A8_UNORM,
		imguiHeap.hCPUHeapStart,
		imguiHeap.hGPUHeapStart);

	auto ec = EngineContext::Context;
	ec->MeshManager = meshManager.get();
	ec->ResourceManager = resourceManager.get();
	ec->ShaderResourceManager = shaderResourceManager.get();
	ec->CommandContext = commandContext.get();
	ec->DeviceResources = deviceResources.get();
	ec->RenderTargetManager = renderTargetManager.get();
	ec->ModelManager = &modelManager;

	renderStages.push_back(std::unique_ptr<IRenderStage>((IRenderStage*)new MainPassRenderStage()));

	auto dir = XMVector3Normalize(XMVectorSet(1, -1, 1, 0));
	XMStoreFloat3(&lightBuffer.DirLight.Direction, dir);
	lightBuffer.DirLight.Color = XMFLOAT3(0.9f, 0.9f, 0.9f);
	lightBuffer.PointLight.Color = XMFLOAT3(0.9f, 0.1f, 0.1f);
	lightBuffer.PointLight.Position = XMFLOAT3(2.9f, 0.1f, 0.1f);
	lightBuffer.PointLight.Range = 5.f;
	lightBuffer.PointLight.Intensity = 2.f;

	for (auto& stage : renderStages)
	{
		stage->Initialize();
	}
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
}

bool show = false;

void Renderer::Render(const FrameContext& frameContext)
{
	auto camera = frameContext.Camera;
	perObject.View = camera->GetViewTransposed();
	perObject.Projection = camera->GetProjectionTransposed();

	auto worlds = frameContext.WorldMatrices;
	auto drawables = frameContext.Drawables;
	auto drawCount = frameContext.DrawableCount;
	auto imageIndex = backBufferIndex;

	//Copy world matrix to constant buffer
	for (size_t i = 0; i < worlds.size(); ++i)
	{
		perObject.World = worlds[i];
		shaderResourceManager->CopyToCB(imageIndex, { &perObject, sizeof(perObject) }, drawables[i].CBView.Offset);
		renderBucket.Insert(drawables[i]);
	}

	lightBuffer.CameraPosition = camera->Position;

	shaderResourceManager->CopyToCB(imageIndex, { &lightBuffer, sizeof(LightBuffer) }, lightBufferView.Offset);
	offsets = shaderResourceManager->CopyDescriptorsToGPUHeap(imageIndex, frameManager.get()); //TO DO: Copy fixed resources to heap first and only copy dynamic resources per frame

	auto rtId = renderTargets[backBufferIndex];
	auto rtv = renderTargetManager->GetRTVHandle(rtId);
	auto dsv = renderTargetManager->GetDSVHandle(depthStencilId);
	auto commandList = commandContext->GetDefaultCommandList();

	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	commandList->SetGraphicsRootSignature(resourceManager->GetRootSignature(mainRootSignatureID));
	std::array<ID3D12DescriptorHeap*, 1> heaps = { frameManager->GetGPUDescriptorHeap(imageIndex) };
	commandList->SetDescriptorHeaps((UINT)heaps.size(), heaps.data());

	PIXBeginEvent(commandList, PIX_COLOR_DEFAULT, L"Render");

	commandList->SetGraphicsRootDescriptorTable(RootSigCBPixel0, frameManager->GetHandle(imageIndex, offsets.ConstantBufferOffset + lightBufferView.Index));
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
					commandList->DrawIndexedInstanced(mesh.IndexCount, 1, 0, 0, 0);
				}
			}
		}
	}

	auto model = modelManager.GetModel({ 0 });
	for (uint32 i = 0; i < model.Meshes.size(); ++i)
	{
		auto mesh = model.Meshes[i];
		auto materialHandle = model.Materials[i];
		auto material = shaderResourceManager->GetMaterial(materialHandle);
		commandList->SetGraphicsRootDescriptorTable(RootSigSRVPixel1, frameManager->GetHandle(imageIndex, offsets.MaterialsOffset + material.StartIndex));
		DrawMesh(mesh);
	}

	heaps[0] = imguiHeap.pDescriptorHeap.Get();
	commandList->SetDescriptorHeaps(1, heaps.data());
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	if (show)
		ImGui::ShowDemoWindow(&show);

	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &show);					// Edit bools storing our window open/close state

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("Dir Light Color", (float*)& lightBuffer.DirLight.Color.x); // Edit 3 floats representing a color
		ImGui::DragFloat3("Point Light Pos", (float*)& lightBuffer.PointLight.Position.x);

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
	}


	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
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
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Renderer::EndInitialization()
{
	cbuffer.Initialize(resourceManager.get(), sizeof(PerObjectConstantBuffer), 16);
	auto meshId = meshManager->CreateMesh("../../Assets/Models/sphere.obj", mesh);
	perObjectView = shaderResourceManager->CreateCBV(sizeof(PerObjectConstantBuffer));
	lightBufferView = shaderResourceManager->CreateCBV(sizeof(LightBuffer));
	TextureID textures[2];
	textures[0] = shaderResourceManager->CreateTexture("../../Assets/Textures/rock.jpg");
	textures[1] = shaderResourceManager->CreateTexture("../../Assets/Textures/rockNormals.jpg");
	auto matId = shaderResourceManager->CreateMaterial(textures, 2, defaultPSO, material);
	modelManager.CreateModel("../../Assets/Models/Sponza.fbx");
	//for (int i = 0; i < CFrameBufferCount; ++i)
	//{
	//	offsets = shaderResourceManager->CopyDescriptorsToGPUHeap(i, frameManager.get());
	//}

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
	auto meshView = meshManager->GetMeshView(mesh);
	DrawMesh(meshView);
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

void Renderer::InitializeCommandContext()
{
	commandContext = std::unique_ptr<CommandContext>(new CommandContext());
	commandContext->Initialize(deviceResources.get());
}

void Renderer::CreateRootSignatures()
{
	CD3DX12_DESCRIPTOR_RANGE range[5];
	//view dependent CBV
	range[0].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//light dependent CBV
	range[1].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
	//G-Buffer inputs
	range[2].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 32, 0);
	//per frame CBV
	range[3].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 1);
	//per bone 
	range[4].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 2);

	CD3DX12_ROOT_PARAMETER rootParameters[5];
	rootParameters[RootSigCBVertex0].InitAsDescriptorTable(1, &range[0], D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[RootSigCBPixel0].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootSigSRVPixel1].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[RootSigCBAll1].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[RootSigCBAll2].InitAsDescriptorTable(1, &range[4], D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC descRootSignature;
	descRootSignature.Init(5, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | // we can deny shader stages here for better performance
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS);

	CD3DX12_STATIC_SAMPLER_DESC StaticSamplers[2];
	StaticSamplers[0].Init(0, D3D12_FILTER_ANISOTROPIC);
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
	auto depthBufferId = resourceManager->CreateResource(
		CD3DX12_RESOURCE_DESC::Tex2D(depthFormat, width, height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		&clearVal,
		D3D12_RESOURCE_STATE_DEPTH_WRITE
	);

	auto depthBuffer = resourceManager->GetResource(depthBufferId);
	depthStencilId = renderTargetManager->CreateDepthStencilView(depthBuffer, depthFormat);
}

void Renderer::WaitForPreviousFrame()
{
	auto swapChain = deviceResources->GetSwapChain();
	backBufferIndex = swapChain->GetCurrentBackBufferIndex();
	commandContext->WaitForFrame(backBufferIndex);
}
