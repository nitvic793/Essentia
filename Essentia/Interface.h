#pragma once

#include "BaseComponents.h"
#include "Entity.h"
#include <iostream>
#include <cereal/types/vector.hpp>
#include <cereal/archives/json.hpp>
#include <fstream>

/*
Special Components:
Transform
Mesh
Material
Model
Lights
*/


struct Vector3
{
	float X;
	float Y;
	float Z;

	void operator=(const DirectX::XMFLOAT3& vector)
	{
		X = vector.x;
		Y = vector.y;
		Z = vector.z;
	}
};

struct Vector4
{
	float X;
	float Y;
	float Z;
	float W;
};

struct TransformInterface
{
	Vector3 Position;
	Vector3 Scale;
	Vector3 Rotation;
};

struct MeshInterface
{
	std::string_view Name;
};

struct EntityInterface
{
	EntityHandle				Handle;
	std::vector<std::string>	Components;
};

struct MaterialInterface
{
	std::vector<std::string_view> Textures;
};

struct Scene
{
	std::vector<EntityInterface>	Entities;
};

struct ResourcePack
{
	std::vector<std::string> Textures;
	std::vector<std::string> CubeMaps;
	std::vector<std::string> Meshes;
	std::vector<std::string> Models;

	template<class Archive>
	void serialize(Archive& archive)
	{
		archive(
			CEREAL_NVP(Textures),
			CEREAL_NVP(CubeMaps),
			CEREAL_NVP(Meshes),
			CEREAL_NVP(Models)
		);
	}
};

struct Level
{
	ResourcePack Resources;

};