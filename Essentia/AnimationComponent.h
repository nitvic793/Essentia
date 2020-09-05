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
	std::vector<BoneInfo>		BoneInfoList;
	uint32						CurrentAnimationIndex = 0;
	std::string					CurrentAnimation;
	float						TotalTime = 0.f;
	float						AnimationSpeed = 1.f;

	static AnimationComponent Create(MeshHandle mesh)
	{
		AnimationComponent component = {};
		const AnimationData& animData = GContext->MeshManager->GetAnimationData(mesh);
		component.BoneInfoList = animData.BoneInfoList;
		component.CurrentAnimation = animData.Animations.GetAnimationName(component.CurrentAnimationIndex); // Default animation is 0
		return component;
	}
	
	template<class Archive>
	void save(Archive& archive) const
	{
	};

	template<class Archive>
	void load(Archive& archive)
	{
	};
};