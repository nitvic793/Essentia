#include "pch.h"
#include "Mesh.h"
#include "ModelLoader.h"
#include "Engine.h"

using namespace DirectX;
using namespace Microsoft::WRL;

MeshHandle MeshManager::CreateMesh(const std::string& filename, MeshView& meshView)
{
	auto meshStringID = String::ID(filename.c_str());
	MeshHandle handle;
	if (meshMap.find(meshStringID) != meshMap.end())
	{
		handle.Id = meshMap[meshStringID];
	}
	else
	{
		auto meshData = ModelLoader::Load(filename);
		handle = CreateMesh(meshData, meshView);
		meshMap[meshStringID] = handle.Id;
	}

	meshNameMap[handle.Id] = filename;
	return handle;
}

MeshHandle MeshManager::CreateMesh(const MeshData& meshData, MeshView& meshView)
{
	MeshBuffer buffer;
	auto device = context->GetDevice();
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12CommandAllocator> allocator;
	context->CreateAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.ReleaseAndGetAddressOf());
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList));
	MeshHandle mesh;

	ComPtr<ID3D12Resource> vBufferUploadHeap;
	ComPtr<ID3D12Resource> iBufferUploadHeap;

	mesh.Id = (uint32)meshes.size();

	uint32 vBufferSize = sizeof(Vertex) * (uint32)meshData.Vertices.size();

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&buffer.VertexBuffer));

	buffer.VertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<const BYTE*>(meshData.Vertices.data());
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	UpdateSubresources(commandList.Get(), buffer.VertexBuffer.Get(), vBufferUploadHeap.Get(), 0, 0, 1, &vertexData);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));
	uint32 iBufferSize = sizeof(uint32) * (uint32)meshData.Indices.size();

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(iBufferSize),
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&buffer.IndexBuffer));

	buffer.IndexBuffer->SetName(L"Index Buffer Resource Heap");

	device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE, // no flags
		&CD3DX12_RESOURCE_DESC::Buffer(vBufferSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<const BYTE*>(meshData.Indices.data());
	indexData.RowPitch = iBufferSize;
	indexData.SlicePitch = iBufferSize;

	UpdateSubresources(commandList.Get(), buffer.IndexBuffer.Get(), iBufferUploadHeap.Get(), 0, 0, 1, &indexData);
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(buffer.IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER));

	meshView.VertexBufferView.BufferLocation = buffer.VertexBuffer->GetGPUVirtualAddress();
	meshView.VertexBufferView.StrideInBytes = sizeof(Vertex);
	meshView.VertexBufferView.SizeInBytes = vBufferSize;

	meshView.IndexBufferView.BufferLocation = buffer.IndexBuffer->GetGPUVirtualAddress();
	meshView.IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	meshView.IndexBufferView.SizeInBytes = iBufferSize;

	meshView.IndexCount = (uint32)meshData.Indices.size();

	context->SubmitCommands(commandList.Get());
	context->WaitForFrame();
	BoundingOrientedBox meshBounds;
	BoundingOrientedBox::CreateFromPoints(meshBounds, meshData.Vertices.size(), (const XMFLOAT3*)meshData.Vertices.data(), sizeof(Vertex));

	meshView.MeshEntries = meshData.MeshEntries;
	bounds.push_back(meshBounds);
	meshes.push_back(meshData);
	buffers.push_back(buffer);
	views.push_back(meshView);

	return mesh;
}

void MeshManager::Initialize(CommandContext* commandContext)
{
	context = commandContext;
}

const MeshView& MeshManager::GetMeshView(MeshHandle handle)
{
	return views[handle.Id];
}

const MeshView& MeshManager::GetMeshView(const char* filename)
{
	auto strId = String::ID(filename);
	MeshView view;
	if (meshMap.find(strId) == meshMap.end())
	{
		CreateMesh(filename, view);
	}

	return GetMeshView({ meshMap[strId] });
}

MeshHandle MeshManager::GetMeshHandle(const char* filename)
{
	auto strId = String::ID(filename);
	MeshView view;
	if (meshMap.find(strId) == meshMap.end())
	{
		return CreateMesh(filename, view);
	}

	return { meshMap[strId] };
}

const DirectX::BoundingOrientedBox& MeshManager::GetBoundingBox(MeshHandle handle)
{
	return bounds[handle.Id];
}

const char* MeshManager::GetName(MeshHandle handle)
{
	return meshNameMap[handle.Id].c_str();
}

void ModelManager::Initialize(ShaderResourceManager* srManager)
{
	shaderResourceManager = srManager;
}

ModelHandle ModelManager::CreateModel(const char* filename)
{
	auto modelData = ModelLoader::LoadModel(filename);
	Model model = {};
	ModelHandle modelHandle;
	modelHandle.Id = (uint32)models.size();

	model.Mesh = es::CreateMesh(modelData.MeshData);

	TextureID textures[MaterialTextureCount];
	std::string assetDirectory = "../../Assets/Textures/";
	for (auto& material : modelData.Materials)
	{
		MaterialHandle materialHandle;
		auto diffuseTex = assetDirectory + material.Diffuse;
		auto normalsTex = assetDirectory + material.Normal;
		auto metalnessTex = assetDirectory + material.Metalness;
		auto roughnessTex = assetDirectory + material.Roughness;
		textures[DiffuseID] = material.Diffuse.empty() ? Default::DefaultDiffuse : shaderResourceManager->CreateTexture(diffuseTex.replace(diffuseTex.size() - 3, 3, "DDS"), DDS, true);
		textures[NormalsID] = material.Normal.empty() ? Default::DefaultNormals : shaderResourceManager->CreateTexture(normalsTex.replace(normalsTex.size() - 3, 3, "DDS"), DDS, true);
		textures[RoughnessID] = material.Roughness.empty() ? Default::DefaultRoughness : shaderResourceManager->CreateTexture(roughnessTex.replace(roughnessTex.size() - 3, 3, "DDS"), DDS, true);
		textures[MetalnessID] = material.Metalness.empty() ? Default::DefaultMetalness : shaderResourceManager->CreateTexture(metalnessTex.replace(metalnessTex.size() - 3, 3, "DDS"), DDS, true);
		std::string matName = "";
		for (int i = 0; i < MaterialTextureCount; ++i)
		{
			matName += std::to_string(textures[i]);
		}

		Material out;
		materialHandle = shaderResourceManager->CreateMaterial(textures, MaterialTextureCount, Default::DefaultMaterialPSO, out, matName.c_str());
		model.Materials.push_back(materialHandle);
	}

	models.push_back(model);
	return modelHandle;
}

const Model& ModelManager::GetModel(ModelHandle model)
{
	return models[model.Id];
}
