#include "pch.h"
#include "Engine.h"
#include "Entity.h"
#include "Renderer.h"

EngineContext* EngineContext::Context = nullptr;

EntityHandle Es::CreateEntity(const Transform& transform)
{
	auto ec = EngineContext::Context;
	return ec->EntityManager->CreateEntity(transform);
}

TextureID Es::CreateTexture(const std::string& filename, TextureType type, bool generateMips)
{
	auto ec = EngineContext::Context;
	return ec->ShaderResourceManager->CreateTexture(filename, type, generateMips);
}

MaterialHandle Es::CreateMaterial(TextureID* textures, uint32 count, PipelineStateID psoID)
{
	auto ec = EngineContext::Context;
	Material mat;
	return ec->ShaderResourceManager->CreateMaterial(textures, count, psoID, mat);
}

MeshHandle Es::CreateMesh(const std::string& filename)
{
	auto ec = EngineContext::Context;
	MeshView mesh;
	return ec->MeshManager->CreateMesh(filename, mesh);
}

PipelineStateID Es::CreatePSO(const std::string& vertexShader, const std::string& pixelShader)
{
	assert(false);
	return PipelineStateID();
}

ConstantBufferView Es::CreateConstantBufferView(size_t sizeInBytes)
{
	auto ec = EngineContext::Context;
	return ec->ShaderResourceManager->CreateCBV((uint32)sizeInBytes);
}

ResourceID Es::CreateGPUResource(const D3D12_RESOURCE_DESC& desc, D3D12_CLEAR_VALUE* clearVal, D3D12_RESOURCE_STATES initialResourceState, D3D12_HEAP_FLAGS heapFlags, CD3DX12_HEAP_PROPERTIES heapProperties)
{
	assert(false);
	return ResourceID();
}

const Material& Es::GetMaterial(MaterialHandle handle)
{
	auto ec = EngineContext::Context;
	return ec->ShaderResourceManager->GetMaterial(handle);
}

const MeshView& Es::GetMeshView(MeshHandle handle)
{
	auto ec = EngineContext::Context;
	return ec->MeshManager->GetMeshView(handle);
}

ID3D12PipelineState* Es::GetPSO(PipelineStateID psoID)
{
	auto ec = EngineContext::Context;
	return ec->ResourceManager->GetPSO(psoID);
}
