#include "Renderer.h"
#include "Window.h"
#include "DeviceResources.h"
#include <wrl.h>
#include "d3dx12.h"
#include <thread>
#include "ShaderManager.h"
#include "InputLayout.h"

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
		renderTargets[i] = renderTargetManager->CreateRenderTargetView(renderTargetBuffers[i].Get());
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

	meshManager = std::unique_ptr<MeshManager>(new MeshManager());
	meshManager->Initialize(commandContext.get());
	cbuffer.Initialize(resourceManager.get(), sizeof(PerObjectConstantBuffer), sizeof(PerObjectConstantBuffer));
}

void Renderer::Clear()
{
	WaitForPreviousFrame();
	auto commandAllocator = commandContext->GetAllocator(backBufferIndex);
	auto commandList = commandContext->GetDefaultCommandList();

	commandContext->ResetAllocator(commandAllocator);
	commandContext->ResetCommandList(commandList, commandAllocator);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargetBuffers[backBufferIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	auto rtId = renderTargets[backBufferIndex];
	auto rtv = renderTargetManager->GetRTVHandle(rtId);
	auto dsv = renderTargetManager->GetDSVHandle(depthStencilId);

	const float clearColor[] = { 0.0f, 0.1f, 0.2f, 0.0f };
	commandList->OMSetRenderTargets(1, &rtv, FALSE, &dsv);
	commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsv, D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
}

void Renderer::Render(const FrameContext& frameContext)
{
	auto camera = frameContext.Camera;
	auto world = DirectX::XMMatrixIdentity();
	XMFLOAT4X4 model;
	XMStoreFloat4x4(&model, XMMatrixTranspose(world));

	perObject.World = model;
	perObject.View = camera->GetViewTransposed();
	perObject.Projection = camera->GetProjectionTransposed();
	cbuffer.CopyData(&perObject, sizeof(perObject));

	auto commandList = commandContext->GetDefaultCommandList();
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->SetGraphicsRootSignature(resourceManager->GetRootSignature(mainRootSignatureID));
	commandList->SetPipelineState(resourceManager->GetPSO(defaultPSO));
	commandList->IASetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY::D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &mesh.VertexBufferView);
	commandList->IASetIndexBuffer(&mesh.IndexBufferView);
//	commandList->SetGraphicsRootConstantBufferView(0, cbuffer.GetIndex(0));
	commandList->DrawIndexedInstanced(mesh.IndexCount, 1, 0, 0, 0);
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
}

void Renderer::EndInitialization()
{
	meshManager->CreateMesh("../../Assets/Models/cube.obj", mesh);
	auto commandList = commandContext->GetDefaultCommandList();
	commandContext->SubmitCommands(commandList);
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
	rootParameters[0].InitAsConstantBufferView(0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
	rootParameters[1].InitAsDescriptorTable(1, &range[1], D3D12_SHADER_VISIBILITY_PIXEL);
	rootParameters[2].InitAsDescriptorTable(1, &range[2], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[3].InitAsDescriptorTable(1, &range[3], D3D12_SHADER_VISIBILITY_ALL);
	rootParameters[4].InitAsDescriptorTable(1, &range[4], D3D12_SHADER_VISIBILITY_ALL);

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
	//psoDesc.DepthStencilState.DepthEnable = false;
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
	commandContext->WaitForFrame();
}
