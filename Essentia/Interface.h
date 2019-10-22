#pragma once

#include "BaseComponents.h"
#include "Entity.h"
#include <cereal/cereal.hpp>
#include <visit_struct/visit_struct.hpp>

/*
Special Components:
Transform
Mesh
Material
Model
Lights
*/

namespace es::interfaces
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

	struct Mesh
	{
		std::string_view Name;
	}; 

	struct Entity
	{
		Transform					Transform;
		std::vector<std::string>	Components;
	};

	struct Material
	{
		std::vector<std::string_view> Textures;
	};

	struct Scene
	{
		std::vector<Entity>	Entities;
	};

	struct Resources
	{
		std::vector<std::string> Textures;
		std::vector<std::string> CubeMaps;
		std::vector<std::string> Meshes;
	};
}