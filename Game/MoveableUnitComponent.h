#pragma once

#include "ComponentReflector.h"

struct MoveableUnitComponent : public IComponent
{
	DirectX::XMFLOAT3 TargetPos = DirectX::XMFLOAT3(0.f, 0.f, 0.f);
	float MoveSpeed = 5.f;

	template<class Archive>
	void save(Archive& archive) const
	{
	};

	template<class Archive>
	void load(Archive& archive)
	{
	};

	GComponent(MoveableUnitComponent)
};