#pragma once

#include "d3dx12.h"
#include <vector>

constexpr uint32 CMaxD3DResources = 256;
constexpr uint32 CMaxPipelineStates = 96;
constexpr uint32 CMaxRootSignatures = 24;

typedef uint32 PipelineStateID;
typedef uint32 RootSignatureID;
typedef uint32 ResourceID;

class ResourceManager
{
public:
	void Initialize(ID3D12Device* device);
	RootSignatureID CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc, D3D_ROOT_SIGNATURE_VERSION version = D3D_ROOT_SIGNATURE_VERSION_1_0);
	PipelineStateID CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);
	ResourceID		CreateResource(D3D12_RESOURCE_DESC desc, D3D12_CLEAR_VALUE* clearVal, D3D12_RESOURCE_STATES initialResourceState,
		D3D12_HEAP_FLAGS heapFlags = D3D12_HEAP_FLAG_NONE, 
		CD3DX12_HEAP_PROPERTIES heapProperties = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT));

	ID3D12PipelineState*	GetPSO(PipelineStateID psoID);
	ID3D12RootSignature*	GetRootSignature(RootSignatureID rsID);
	ID3D12Resource*			GetResource(ResourceID resourceID);
private:
	ResourceManager() {}
	ID3D12Device* device = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>>	pipelineStates;
	std::vector<Microsoft::WRL::ComPtr<ID3D12RootSignature>>	rootSignatures;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>>			resources;
	PipelineStateID currentPSOID = 0;
	RootSignatureID currentRSID = 0;
	ResourceID		currentResourceID = 0;
	friend class Renderer;
};
