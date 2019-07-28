#pragma once

#include "EngineContext.h"

namespace Es
{
	EntityHandle			CreateEntity(const Transform& transform = DefaultTransform);
	TextureID				CreateTexture(const std::string& filename, TextureType type, bool generateMips = true);
	MaterialHandle			CreateMaterial(TextureID* textures, uint32 count, PipelineStateID psoID);
	MeshHandle				CreateMesh(const std::string& filename);
	PipelineStateID			CreatePSO(const std::string& vertexShader, const std::string& pixelShader);
	ConstantBufferView		CreateConstantBufferView(size_t sizeInBytes);
	ResourceID				CreateGPUResource(
		const D3D12_RESOURCE_DESC& desc, 
		D3D12_CLEAR_VALUE* clearVal, 
		D3D12_RESOURCE_STATES initialResourceState,
		D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE,
		CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT)
	);

	const Material&			GetMaterial(MaterialHandle handle);
	const MeshView&			GetMeshView(MeshHandle handle);
	ID3D12PipelineState*	GetPSO(PipelineStateID psoID);
}