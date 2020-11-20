#include "pch.h"
#include "Mesh.h"
#include "ModelLoader.h"
#include "Engine.h"

using namespace DirectX;
using namespace Microsoft::WRL;

MeshHandle MeshManager::CreateMesh(const std::string& filename, MeshView& meshView)
{
	auto meshStringID = String::ID(filename.c_str());
	MeshHandle handle = { (uint32)-1 }; // Default is invalid
	if (meshMap.find(meshStringID) != meshMap.end())
	{
		handle.Id = meshMap[meshStringID];
	}
	else
	{
		auto meshData = ModelLoader::Load(filename);
		if (meshData.Vertices.size() == 0)
		{
			return handle;
		}
		handle = CreateMesh(meshData, meshView);
		meshMap[meshStringID] = handle.Id;
		if (meshData.IsAnimated)
		{
			CreateBoneBuffers(handle, meshData.AnimationData);
		}
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

	auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);

	device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vBufferResourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&buffer.VertexBuffer));

	buffer.VertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vBufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<const BYTE*>(meshData.Vertices.data());
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	UpdateSubresources(commandList.Get(), buffer.VertexBuffer.Get(), vBufferUploadHeap.Get(), 0, 0, 1, &vertexData);

	auto transitionVBuffer = CD3DX12_RESOURCE_BARRIER::Transition(buffer.VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandList->ResourceBarrier(1, &transitionVBuffer);

	uint32 iBufferSize = sizeof(uint32) * (uint32)meshData.Indices.size();
	auto iBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);

	device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&iBufferResourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&buffer.IndexBuffer));

	buffer.IndexBuffer->SetName(L"Index Buffer Resource Heap");

	device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE, // no flags
		&iBufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	iBufferUploadHeap->SetName(L"Index Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<const BYTE*>(meshData.Indices.data());
	indexData.RowPitch = iBufferSize;
	indexData.SlicePitch = iBufferSize;

	UpdateSubresources(commandList.Get(), buffer.IndexBuffer.Get(), iBufferUploadHeap.Get(), 0, 0, 1, &indexData);
	auto transitionIBuffer = CD3DX12_RESOURCE_BARRIER::Transition(buffer.IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandList->ResourceBarrier(1, &transitionIBuffer);

	meshView.VertexBufferView.BufferLocation = buffer.VertexBuffer->GetGPUVirtualAddress();
	meshView.VertexBufferView.StrideInBytes = sizeof(Vertex);
	meshView.VertexBufferView.SizeInBytes = vBufferSize;

	meshView.IndexBufferView.BufferLocation = buffer.IndexBuffer->GetGPUVirtualAddress();
	meshView.IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	meshView.IndexBufferView.SizeInBytes = iBufferSize;

	meshView.IndexCount = (uint32)meshData.Indices.size();

	context->SubmitCommands(commandList.Get());
	context->WaitForGPU(GContext->DeviceResources->GetSwapChain()->GetCurrentBackBufferIndex()); // Todo: Create function which gets back buffer index automatically
	BoundingBox meshBounds;
	BoundingBox::CreateFromPoints(meshBounds, meshData.Vertices.size(), (const XMFLOAT3*)&meshData.Vertices.data()->Position, sizeof(Vertex));

	meshView.MeshEntries = meshData.MeshEntries;
	bounds.push_back(meshBounds);
	meshes.push_back(meshData);
	buffers.push_back(buffer);
	views.push_back(meshView);

	return mesh;
}

MeshHandle MeshManager::CreateMesh(MeshData& meshData, MeshView& meshView, const char* meshName)
{
	auto meshStringID = String::ID(meshName);
	MeshHandle handle = { (uint32)-1 }; // Default is invalid
	if (meshMap.find(meshStringID) != meshMap.end())
	{
		return handle;
	}
	else
	{
		if (meshData.Vertices.size() == 0)
		{
			return handle;
		}
		handle = CreateMesh(meshData, meshView);
		meshMap[meshStringID] = handle.Id;
		if (meshData.IsAnimated)
		{
			CreateBoneBuffers(handle, meshData.AnimationData);
		}
	}

	meshNameMap[handle.Id] = meshName;
	return handle;
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

const DirectX::BoundingBox& MeshManager::GetBoundingBox(MeshHandle handle)
{
	return bounds[handle.Id];
}

std::string MeshManager::GetName(MeshHandle handle)
{
	return meshNameMap[handle.Id];
}

std::vector<std::string> MeshManager::GetAllMeshNames()
{
	std::vector<std::string> meshNames;
	for (auto mesh : meshNameMap)
	{
		meshNames.push_back(mesh.second);
	}
	return meshNames;
}

bool MeshManager::IsAnimated(MeshHandle mesh) const
{
	return meshes[mesh.Id].IsAnimated;
}

const AnimationData& MeshManager::GetAnimationData(MeshHandle mesh)
{
	return meshes[mesh.Id].AnimationData;
}

void MeshManager::UpdateMeshData(MeshHandle mesh, const MeshData& meshData)
{
	MeshBuffer& buffer = buffers[mesh.Id];
	auto device = context->GetDevice();
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12CommandAllocator> allocator;
	context->CreateAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.ReleaseAndGetAddressOf());
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

	uint32 vBufferSize = sizeof(Vertex) * (uint32)meshData.Vertices.size();
	uint32 iBufferSize = sizeof(uint32) * (uint32)meshData.Indices.size();

	ComPtr<ID3D12Resource> vBufferUploadHeap;
	ComPtr<ID3D12Resource> iBufferUploadHeap;
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);
	auto iBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(iBufferSize);

	device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vBufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));
	vBufferUploadHeap->SetName(L"Terrain Vertex Buffer Upload Resource Heap");

	device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE, // no flags
		&iBufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&iBufferUploadHeap));
	iBufferUploadHeap->SetName(L"Terrain Index Buffer Upload Resource Heap");

	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<const BYTE*>(meshData.Vertices.data());
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	auto transitionVBufferCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(buffer.VertexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(1, &transitionVBufferCopyDest);
	UpdateSubresources(commandList.Get(), buffer.VertexBuffer.Get(), vBufferUploadHeap.Get(), 0, 0, 1, &vertexData);
	auto transitionVBuffer = CD3DX12_RESOURCE_BARRIER::Transition(buffer.VertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandList->ResourceBarrier(1, &transitionVBuffer);

	D3D12_SUBRESOURCE_DATA indexData = {};
	indexData.pData = reinterpret_cast<const BYTE*>(meshData.Indices.data());
	indexData.RowPitch = iBufferSize;
	indexData.SlicePitch = iBufferSize;

	auto transitionIBufferCopyDest = CD3DX12_RESOURCE_BARRIER::Transition(buffer.IndexBuffer.Get(), D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, D3D12_RESOURCE_STATE_COPY_DEST);
	commandList->ResourceBarrier(1, &transitionIBufferCopyDest);
	UpdateSubresources(commandList.Get(), buffer.IndexBuffer.Get(), iBufferUploadHeap.Get(), 0, 0, 1, &indexData);
	auto transitionIBuffer = CD3DX12_RESOURCE_BARRIER::Transition(buffer.IndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandList->ResourceBarrier(1, &transitionIBuffer);

	BoundingBox meshBounds;
	BoundingBox::CreateFromPoints(meshBounds, meshData.Vertices.size(), (const XMFLOAT3*)&meshData.Vertices.data()->Position, sizeof(Vertex));

	this->meshes[mesh.Id] = meshData;
	this->bounds[mesh.Id] = meshBounds;

	auto commandQueue = GContext->DeviceResources->GetCommandQueue();
	auto backbufferIndex = context->GetBackbufferIndex();
	context->SubmitCommands(commandList.Get());
	context->WaitForGPU(backbufferIndex);
	//context->WaitForFrame();
}

void MeshManager::CreateBoneBuffers(MeshHandle meshHandle, AnimationData& animData)
{
	MeshView& meshView = views[meshHandle.Id];
	MeshBuffer& meshBuffer = buffers[meshHandle.Id];
	UINT vBufferSize = sizeof(VertexBoneData) * (UINT)animData.Bones.size();;

	auto device = context->GetDevice();
	ComPtr<ID3D12GraphicsCommandList> commandList;
	ComPtr<ID3D12CommandAllocator> allocator;
	ComPtr<ID3D12Resource> vBufferUploadHeap;

	auto defaultHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto uploadHeapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto vBufferResourceDesc = CD3DX12_RESOURCE_DESC::Buffer(vBufferSize);

	context->CreateAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.ReleaseAndGetAddressOf());
	device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, allocator.Get(), nullptr, IID_PPV_ARGS(&commandList));

	device->CreateCommittedResource(
		&defaultHeapProperties,
		D3D12_HEAP_FLAG_NONE, // no flags
		&vBufferResourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&meshBuffer.BoneVertexBuffer));

	meshBuffer.BoneVertexBuffer->SetName(L"Vertex Buffer Resource Heap");

	device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&vBufferResourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vBufferUploadHeap));

	vBufferUploadHeap->SetName(L"Vertex Buffer Upload Resource Heap");
	D3D12_SUBRESOURCE_DATA vertexData = {};
	vertexData.pData = reinterpret_cast<BYTE*>(animData.Bones.data());
	vertexData.RowPitch = vBufferSize;
	vertexData.SlicePitch = vBufferSize;

	UpdateSubresources(commandList.Get(), meshBuffer.BoneVertexBuffer.Get(), vBufferUploadHeap.Get(), 0, 0, 1, &vertexData);

	meshView.BoneVertexBufferView.BufferLocation = meshBuffer.BoneVertexBuffer->GetGPUVirtualAddress();
	meshView.BoneVertexBufferView.StrideInBytes = sizeof(VertexBoneData);
	meshView.BoneVertexBufferView.SizeInBytes = vBufferSize;

	auto transitionResource = CD3DX12_RESOURCE_BARRIER::Transition(meshBuffer.BoneVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	commandList->ResourceBarrier(1, &transitionResource);
	context->SubmitCommands(commandList.Get());
	context->WaitForGPU(GContext->DeviceResources->GetSwapChain()->GetCurrentBackBufferIndex());
}

void ModelManager::Initialize(ShaderResourceManager* srManager)
{
	shaderResourceManager = srManager;
}

ModelHandle ModelManager::CreateModel(const char* filename)
{
	auto stringID = String::ID(filename);

	if (modelMap.find(stringID) != modelMap.end())
	{
		return modelMap[stringID];
	}

	auto modelData = ModelLoader::LoadModel(filename);
	Model model = {};
	ModelHandle modelHandle;
	modelHandle.Id = (uint32)models.size();

	model.Mesh = es::CreateMesh(modelData.MeshData);

	TextureID textures[MaterialTextureCount];
	std::string assetDirectory = "Assets/Textures/";
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

	auto meshView = EngineContext::Context->MeshManager->GetMeshView(model.Mesh);
	for (auto entry : meshView.MeshEntries)
	{
		BoundingBox meshBounds;
		BoundingBox::CreateFromPoints(meshBounds, entry.NumIndices, (const XMFLOAT3*)&(modelData.MeshData.Vertices.data() + entry.NumIndices)->Position, sizeof(Vertex));
		model.Bounds.push_back(meshBounds);
	}
	models.push_back(model);
	modelNameMap[modelHandle.Id] = filename;
	modelMap[stringID] = modelHandle;
	return modelHandle;
}

const Model& ModelManager::GetModel(ModelHandle model)
{
	return models[model.Id];
}

ModelHandle ModelManager::GetModelHandle(const char* name)
{
	return modelMap[String::ID(name)];
}

std::string ModelManager::GetModelName(ModelHandle model)
{
	return modelNameMap[model.Id];
}

std::vector<std::string> ModelManager::GetAllModelNames()
{
	std::vector<std::string> modelNames;
	for (auto model : modelNameMap)
	{
		modelNames.push_back(model.second);
	}

	return modelNames;
}
