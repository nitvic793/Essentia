#pragma once
#include <DirectXMath.h>
#include "EntityBase.h"
#include "Memory.h"

struct TransformRef
{
	DirectX::XMFLOAT3* Position;
	DirectX::XMFLOAT3* Rotation;
	DirectX::XMFLOAT3* Scale;
};

struct Transform
{
	DirectX::XMFLOAT3 Position;
	DirectX::XMFLOAT3 Rotation;
	DirectX::XMFLOAT3 Scale;
};

constexpr Transform DefaultTransform = { {0,0,0}, {0,0,0}, {1,1,1} };

struct TransformHandle
{
	int Index;
};

constexpr bool IsValidParent(TransformHandle handle)
{
	return (handle.Index != -1);
}

struct TransformData
{
	uint32	Size;
	uint32	Capacity;
	byte* Buffer;

	DirectX::XMFLOAT4X4* World;
	DirectX::XMFLOAT4X4* Local;
	TransformHandle* Parent;
	EntityHandle* Entity;
};

// Transform Index = Entity Index
class TransformManager
{
public:
	TransformManager();
	TransformHandle		CreateTransform(EntityHandle entity, TransformHandle parent = { -1 }, const Transform& transform = DefaultTransform);
	void				SetLocal(EntityHandle entity, const Transform& transform);
	EntityHandle		GetParent(EntityHandle entity);
	DirectX::XMFLOAT4X4 GetTransposedWorldMatrix(EntityHandle entity);
	DirectX::XMFLOAT4X4 GetWorldMatrix(EntityHandle entity);
	const Transform		GetWorldTransform(EntityHandle entity);
	~TransformManager();
private:
	void UpdateTransform(TransformHandle transform);
	void Allocate(uint32 count);
	void CleanUp();
	TransformData transforms;
};