#pragma once

#include "EngineContext.h"
#include "EntityBase.h"
#include "ConstantBuffer.h"
#include "Animation.h"
#include "Mesh.h"

struct AnimationComponent : public IComponent
{	
	GComponent(AnimationComponent)

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

	static AnimationComponent Create(MeshHandle mesh)
	{
		AnimationComponent component = {};
		component.Mesh = mesh;
		component.ArmatureCBV = es::CreateConstantBufferView(sizeof(PerArmatureConstantBuffer));
		const AnimationData& animData = GContext->MeshManager->GetAnimationData(mesh);
		memcpy(&component.BoneInfoList[0], animData.BoneInfoList.data(), animData.BoneInfoList.size() * sizeof(BoneInfo));
		component.BoneInfoSize = (uint32)animData.BoneInfoList.size();
		component.CurrentAnimation = animData.Animations.GetAnimationName(component.CurrentAnimationIndex); // Default animation is 0
		return component;
	}
	
	template<class Archive>
	void save(Archive& archive) const
	{
		archive(
			CEREAL_NVP(CurrentAnimationIndex),
			CEREAL_NVP(Mesh.Id)
		);
	};

	template<class Archive>
	void load(Archive& archive)
	{
		archive(
			CEREAL_NVP(CurrentAnimationIndex),
			CEREAL_NVP(Mesh.Id)
		);
		AnimationComponent component = Create(Mesh);
		component.CurrentAnimationIndex = CurrentAnimationIndex;
		*this = component;
	};
};