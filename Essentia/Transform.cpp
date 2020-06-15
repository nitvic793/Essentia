#include "pch.h"
#include "Transform.h"
#include "Memory.h"

constexpr int32 CRootNode = -1;

using namespace DirectX;

XMFLOAT4X4 GetTransformMatrix(const XMFLOAT4X4& local, const XMFLOAT4X4& parent)
{
	auto transformation = XMLoadFloat4x4(&local);
	auto parentTransform = XMLoadFloat4x4(&parent);
	transformation = transformation * parentTransform;

	XMFLOAT4X4 outTransform;
	XMStoreFloat4x4(&outTransform, transformation);
	return outTransform;
}

XMFLOAT4X4 GetTransformMatrix(const Transform& transform)
{
	auto pos = XMLoadFloat3(&transform.Position);
	auto rot = XMLoadFloat3(&transform.Rotation);
	auto scaleV = XMLoadFloat3(&transform.Scale);

	auto translation = XMMatrixTranslationFromVector(pos);
	auto rotation = XMMatrixRotationRollPitchYawFromVector(rot);
	auto scale = XMMatrixScalingFromVector(scaleV);
	auto transformation = scale * rotation * translation;

	XMFLOAT4X4 transformMatrix;
	XMStoreFloat4x4(&transformMatrix, transformation);
	return transformMatrix;
}

TransformManager::TransformManager()
{
	transforms = {};
	Allocate(CMaxInitialEntityCount);
}

TransformHandle TransformManager::CreateTransform(EntityHandle entity, TransformHandle parent, const Transform& transform)
{
	int index = (int)entity.Handle.Index;
	TransformHandle handle = { index };
	transforms.Size++;
	transforms.Local[index] = GetTransformMatrix(transform);
	transforms.Entity[index] = entity;
	transforms.Parent[index] = parent;
	UpdateTransform(handle);
	return handle;
}

void TransformManager::SetLocal(EntityHandle entity, const Transform& transform)
{
	TransformHandle handle = { (int)entity.Handle.Index }; //Entity Index directly maps to Transform Index
	transforms.Local[handle.Index] = GetTransformMatrix(transform);
	UpdateTransform(handle);
}

EntityHandle TransformManager::GetParent(EntityHandle entity)
{
	auto parentIndex = transforms.Parent[entity.Handle.Index];
	return transforms.Entity[parentIndex.Index];;
}

DirectX::XMFLOAT4X4 TransformManager::GetTransposedWorldMatrix(EntityHandle entity)
{
	auto world = XMLoadFloat4x4(&transforms.World[entity.Handle.Index]);
	XMFLOAT4X4 outTransposedWorld;
	XMStoreFloat4x4(&outTransposedWorld, XMMatrixTranspose(world));
	return outTransposedWorld;
}

DirectX::XMFLOAT4X4 TransformManager::GetWorldMatrix(EntityHandle entity)
{
	return transforms.World[entity.Handle.Index];
}

const Transform TransformManager::GetWorldTransform(EntityHandle entity)
{
	int32 index = (int32)entity.Handle.Index;
	auto world = transforms.World[index];
	XMVECTOR pos;
	XMVECTOR rot;
	XMVECTOR scale;
	XMMatrixDecompose(&scale, &rot, &pos, XMLoadFloat4x4(&world));
	Transform outTransform;
	XMVECTOR rotation = XMVectorSet(0, 0, 0, 0);
	float angle = 0.f;
	XMQuaternionToAxisAngle(&rotation, &angle, rot);

	XMStoreFloat3(&outTransform.Position, pos);
	XMStoreFloat3(&outTransform.Scale, scale);
	XMStoreFloat3(&outTransform.Rotation, rotation);
	return outTransform;
}

const bool TransformManager::HasValidParent(EntityHandle entity)
{
	return transforms.Parent[entity.Handle.Index].Index != CRootNode;
}

Vector<EntityHandle> TransformManager::GetChildren(EntityHandle entity)
{
	uint32 childrenCount = 0;
	for (uint32 i = 0; i < CMaxInitialEntityCount; ++i)
	{
		if (transforms.Parent[i].Index == entity.Handle.Index)
		{
			childrenCount++;
		}
	}

	Vector<EntityHandle> children(childrenCount, Mem::GetFrameAllocator());
	for (uint32 i = 0; i < CMaxInitialEntityCount; ++i)
	{
		if (transforms.Parent[i].Index == entity.Handle.Index)
		{
			children.Push(transforms.Entity[i]);
		}
	}

	return children;
}

TransformManager::~TransformManager()
{
	CleanUp();
}

void TransformManager::UpdateTransform(TransformHandle transform)
{
	auto index = transform.Index;
	if (IsValidParent(transforms.Parent[index]))
	{
		auto parent = transforms.Parent[index];
		transforms.World[index] = GetTransformMatrix(transforms.Local[index], transforms.World[parent.Index]);
	}
	else
	{
		transforms.World[index] = transforms.Local[index];
	}
}

void TransformManager::Allocate(uint32 count)
{
	size_t totalSize = (size_t)count * (sizeof(XMFLOAT4X4) + sizeof(XMFLOAT4X4) + sizeof(TransformHandle) + sizeof(EntityHandle));
	transforms.Buffer = (byte*)Mem::Alloc(totalSize);
	memset(transforms.Buffer, 0, totalSize);
	transforms.World = (XMFLOAT4X4*)transforms.Buffer;
	transforms.Local = (XMFLOAT4X4*)(transforms.World + count);
	transforms.Parent = (TransformHandle*)(transforms.Local + count);
	transforms.Entity = (EntityHandle*)(transforms.Parent + count);
	transforms.Capacity = count;
	for (uint32 i = 0; i < count; ++i)
	{
		transforms.Parent[i].Index = CRootNode;
	}
}

void TransformManager::CleanUp()
{
	Mem::Free(transforms.Buffer);
}
