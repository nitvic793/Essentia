#include "pch.h"
#include "Mesh.h"
#include "ModelLoader.h"

using namespace DirectX;
using namespace Microsoft::WRL;

MeshHandle MeshManager::CreateMesh(const std::string& filename, MeshView& meshView)
{
	auto meshData = ModelLoader::Load(filename);
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
	vertexData.pData = reinterpret_cast<BYTE*>(meshData.Vertices.data());
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
	indexData.pData = reinterpret_cast<BYTE*>(meshData.Indices.data());
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

	context->SubmitCommands(commandList.Get());
	
	BoundingOrientedBox meshBounds;
	BoundingOrientedBox::CreateFromPoints(meshBounds, meshData.Vertices.size(), (const XMFLOAT3*)meshData.Vertices.data(), sizeof(Vertex));

	bounds.push_back(meshBounds);
	meshes.push_back(meshData);
	buffers.push_back(buffer);
	views.push_back(meshView);
	meshView.IndexCount = (uint32)meshData.Indices.size();
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
