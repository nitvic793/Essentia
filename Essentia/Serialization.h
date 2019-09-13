#pragma once

#include "BaseComponents.h"
#include "Entity.h"
#include <cereal/cereal.hpp>

//
namespace Es::serial
{
	struct Vector3
	{
		float X;
		float Y;
		float Z;
	};

	struct Vector4
	{
		float X;
		float Y;
		float Z;
		float W;
	};

	struct Transform
	{
		Vector3 Position;
		Vector3 Scale;
		Vector3 Rotation;
	};

	struct Entity
	{
		Transform Transform;
	};
}