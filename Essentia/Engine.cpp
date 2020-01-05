#include "pch.h"
#include "Engine.h"
#include "Entity.h"
#include "Renderer.h"

EngineContext* EngineContext::Context = nullptr;
EngineContext* GContext = nullptr;

EntityHandle es::CreateEntity(const Transform& transform)
{
	auto ec = EngineContext::Context;
	return ec->EntityManager->CreateEntity(transform);
}

TextureID es::CreateTexture(const std::string& filename, TextureType type, bool generateMips)
{
	auto ec = EngineContext::Context;
	return ec->ShaderResourceManager->CreateTexture(filename, type, generateMips);
}

MaterialHandle es::CreateMaterial(TextureID* textures, uint32 count, PipelineStateID psoID)
{
	auto ec = EngineContext::Context;
	Material mat;
	return ec->ShaderResourceManager->CreateMaterial(textures, count, psoID, mat);
}

MeshHandle es::CreateMesh(const std::string& filename)
{
	auto ec = EngineContext::Context;
	MeshView mesh;
	return ec->MeshManager->CreateMesh(filename, mesh);
}

MeshHandle es::CreateMesh(const MeshData& meshData)
{
	auto ec = EngineContext::Context;
	MeshView mesh;
	return ec->MeshManager->CreateMesh(meshData, mesh);
}

ModelHandle es::CreateModel(const char* filename)
{
	auto ec = EngineContext::Context;
	return ec->ModelManager->CreateModel(filename);
}

PipelineStateID es::CreatePSO(const std::string& vertexShader, const std::string& pixelShader)
{
	assert(false);
	return PipelineStateID();
}

ConstantBufferView es::CreateConstantBufferView(size_t sizeInBytes)
{
	auto ec = EngineContext::Context;
	return ec->ShaderResourceManager->CreateCBV((uint32)sizeInBytes);
}

ResourceID es::CreateGPUResource(const D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE* clearVal, D3D12_RESOURCE_STATES initialResourceState, D3D12_HEAP_FLAGS heapFlags, CD3DX12_HEAP_PROPERTIES heapProperties)
{
	assert(false);
	return ResourceID();
}

const Material& es::GetMaterial(MaterialHandle handle)
{
	auto ec = EngineContext::Context;
	return ec->ShaderResourceManager->GetMaterial(handle);
}

const MeshView& es::GetMeshView(MeshHandle handle)
{
	auto ec = EngineContext::Context;
	return ec->MeshManager->GetMeshView(handle);
}

const Model& es::GetModel(ModelHandle handle)
{
	auto ec = EngineContext::Context;
	return ec->ModelManager->GetModel(handle);
}

ID3D12PipelineState* es::GetPSO(PipelineStateID psoID)
{
	auto ec = EngineContext::Context;
	return ec->ResourceManager->GetPSO(psoID);
}
