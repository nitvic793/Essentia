#include "pch.h"
#include "ResourceManager.h"

void ResourceManager::Initialize(ID3D12Device* device)
{
	this->device = device;
	pipelineStates.resize(CMaxPipelineStates);
	rootSignatures.resize(CMaxRootSignatures);
}

RootSignatureID ResourceManager::CreateRootSignature(const D3D12_ROOT_SIGNATURE_DESC& rootSigDesc, D3D_ROOT_SIGNATURE_VERSION version)
{
	auto id = currentRSID;
	currentRSID++;
	Microsoft::WRL::ComPtr<ID3DBlob> rootSigBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob;
	D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1, &rootSigBlob, &errorBlob);
	device->CreateRootSignature(0, rootSigBlob->GetBufferPointer(), rootSigBlob->GetBufferSize(), IID_PPV_ARGS(rootSignatures[id].ReleaseAndGetAddressOf()));
	return id;
}

PipelineStateID ResourceManager::CreatePSO(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc)
{
	auto id = currentPSOID;
	currentPSOID++;
	device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(pipelineStates[id].ReleaseAndGetAddressOf()));
	return currentPSOID;
}

ID3D12PipelineState* ResourceManager::GetPSO(PipelineStateID psoID)
{
	return pipelineStates[psoID].Get();
}

ID3D12RootSignature* ResourceManager::GetRootSignature(RootSignatureID rsID)
{
	return rootSignatures[rsID].Get();
}
