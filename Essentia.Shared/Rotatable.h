#pragma once

#include "EntityBase.h"

//@Serializable()
struct Rotatable : public IComponent
{
	float Speed = 1.f;
	float Rotation = 0.f;

	template<class Archive>
	void save(Archive& archive) const
	{
	};

	template<class Archive>
	void load(Archive& archive)
	{
	}

	GComponent(Rotatable)
};