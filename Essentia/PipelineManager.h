#pragma once

#include "d3dx12.h"
#include <vector>

constexpr uint32 CMaxD3DResources = 256;
constexpr uint32 CMaxPipelineStates = 96;
constexpr uint32 CMaxRootSignatures = 24;

typedef uint32 PipelineStateID;
typedef uint32 RootSignatureID;

class ResourceManager
{
public:
	void Initialize(ID3D12Device* device);
	RootSignatureID CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc, D3D_ROOT_SIGNATURE_VERSION version = D3D_ROOT_SIGNATURE_VERSION_1_0);
	PipelineStateID CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc);
	ID3D12PipelineState* GetPSO(PipelineStateID psoID);
	ID3D12RootSignature* GetRootSignature(RootSignatureID rsID);
private:
	ResourceManager() {}
	ID3D12Device* device = nullptr;
	std::vector<Microsoft::WRL::ComPtr<ID3D12PipelineState>> pipelineStates;
	std::vector<Microsoft::WRL::ComPtr<ID3D12RootSignature>> rootSignatures;
	PipelineStateID currentPSOID = 0;
	RootSignatureID currentRSID = 0;
	friend class Renderer;
};

