#pragma once

#include "EntityBase.h"
#include "Mesh.h"
#include "Material.h"
#include "Declarations.h"
#include "Engine.h"
#include "ConstantBuffer.h"
#include "CMath.h"


struct PostProcessVolumeComponent : public IComponent
{
	GComponent(PostProcessVolumeComponent)

	template<class Archive>
	void serialize(Archive& archive)
	{
		//archive(
		//);
	};
};