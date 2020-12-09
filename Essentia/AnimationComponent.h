#pragma once

#include "EngineContext.h"
#include "EntityBase.h"
#include "ConstantBuffer.h"
#include "Animation.h"
#include "Mesh.h"

struct AnimationComponent : public IComponent
{	
	GComponent(AnimationComponent)
	std::string					MeshName;
	MeshHandle					Mesh; //Reference Mesh
	PerArmatureConstantBuffer	ArmatureConstantBuffer;
	ConstantBufferView			ArmatureCBV;
	BoneInfo					BoneInfoList[CMaxBones];
	uint32						BoneInfoSize = 0;
	uint32						CurrentAnimationIndex = 0;
	std::string_view			CurrentAnimation;
	float						TotalTime = 0.f;
	float						AnimationSpeed = 1.f;
	bool						IsPlaying = true;

	static AnimationComponent Create(std::string meshName)
	{
		AnimationComponent component = {};
		component.MeshName = meshName;
		component.Mesh = GContext->MeshManager->GetMeshHandle(meshName.c_str());
		component.ArmatureCBV = es::CreateConstantBufferView(sizeof(PerArmatureConstantBuffer));
		const AnimationData& animData = GContext->MeshManager->GetAnimationData(component.Mesh);
		memcpy(&component.BoneInfoList[0], animData.BoneInfoList.data(), animData.BoneInfoList.size() * sizeof(BoneInfo));
		component.BoneInfoSize = (uint32)animData.BoneInfoList.size();
		component.CurrentAnimation = animData.Animations.GetAnimationName(component.CurrentAnimationIndex); // Default animation is 0
		return component;
	}

	static AnimationComponent Create(MeshHandle mesh)
	{
		return Create(GContext->MeshManager->GetName(mesh));
	}
	
	template<class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(CurrentAnimationIndex),
			CEREAL_NVP(MeshName)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(CurrentAnimationIndex),
			CEREAL_NVP(MeshName)
		);

		AnimationComponent component = Create(MeshName);
		component.CurrentAnimationIndex = CurrentAnimationIndex;
		*this = component;
	};
};