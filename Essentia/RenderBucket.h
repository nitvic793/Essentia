#pragma once
#include "Mesh.h"
#include "Material.h"
#include "ShaderResourceManager.h"
#include <unordered_map>
#include "BaseComponents.h"
#include "Engine.h"

struct MeshBucket
{
	MeshView			Mesh;
	std::vector<uint32> CbIndices = {};
	std::vector<D3D12_GPU_VIRTUAL_ADDRESS> vAddresses = {};
};

struct MaterialBucket
{
	Material Material;
	std::unordered_map<uint32, MeshBucket> Instances = {};
};

struct PipelineBucket
{
	ID3D12PipelineState* PipelineStateObject;
	std::unordered_map<uint32, MaterialBucket> Instances = {};
};

struct RenderBucket
{
	std::unordered_map<PipelineStateID, PipelineBucket> Pipelines;

	void Insert(const DrawableComponent& component, D3D12_GPU_VIRTUAL_ADDRESS vAddress)
	{
		auto matHandle = component.Material;
		auto meshHandle = component.Mesh;
		auto cbIndex = component.CBView.Index;
		auto material = Es::GetMaterial(matHandle);
		auto meshView = Es::GetMeshView(meshHandle);

		ID3D12PipelineState* pso = Es::GetPSO(material.PipelineID);

		if (Pipelines.find(material.PipelineID) == Pipelines.end())
		{
			Pipelines[material.PipelineID] = { pso };
		}

		PipelineBucket& bucket = Pipelines[material.PipelineID];
		if (bucket.Instances.find(matHandle.Index) == bucket.Instances.end())
		{
			bucket.Instances[matHandle.Index] = { material };
		}

		MaterialBucket& matBucket = bucket.Instances[matHandle.Index];
		if (matBucket.Instances.find(meshHandle.Id) == matBucket.Instances.end())
		{
			matBucket.Instances[meshHandle.Id] = { meshView };
		}

		auto& meshBucket = matBucket.Instances[meshHandle.Id];
		meshBucket.CbIndices.push_back(cbIndex);
		meshBucket.vAddresses.push_back(vAddress);
	}

	void Clear()
	{
		Pipelines.clear();
	}
};
