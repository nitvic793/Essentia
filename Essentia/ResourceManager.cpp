#include "pch.h"
#include "ResourceManager.h"

void ResourceManager::Initialize(ID3D12Device* device)
{
	this->device = device;
	pipelineStates.resize(CMaxPipelineStates);
	rootSignatures.resize(CMaxRootSignatures);
	resources.resize(CMaxD3DResources);
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
	return id;
}

ResourceID ResourceManager::CreateResource(D3D12_RESOURCE_DESC desc, D3D12_CLEAR_VALUE* clearVal, D3D12_RESOURCE_STATES initialResourceState, D3D12_HEAP_FLAGS heapFlags, CD3DX12_HEAP_PROPERTIES heapProperties)
{
	auto id = currentResourceID;
	currentResourceID++;
	device->CreateCommittedResource(&heapProperties, heapFlags, &desc, initialResourceState, clearVal, IID_PPV_ARGS(resources[id].ReleaseAndGetAddressOf()));
	resources[id]->SetName((std::wstring(L"Resource ") + std::to_wstring(id)).c_str());
	return id;
}

ID3D12PipelineState* ResourceManager::GetPSO(PipelineStateID psoID)
{
	return pipelineStates[psoID].Get();
}

ID3D12RootSignature* ResourceManager::GetRootSignature(RootSignatureID rsID)
{
	return rootSignatures[rsID].Get();
}

ID3D12Resource* ResourceManager::GetResource(ResourceID resourceID)
{
	return resources[resourceID].Get();
}

ID3D12Resource** ResourceManager::RequestEmptyResource(ResourceID& outResourceID)
{
	outResourceID = currentResourceID;
	currentResourceID++;
	return resources[outResourceID].ReleaseAndGetAddressOf();
}

void ResourceManager::Release(ResourceID resourceID)
{
	resources[resourceID].Reset();
}
