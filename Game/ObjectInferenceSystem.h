#pragma once


#include "System.h"
#include "RenderComponents.h"
#include <DirectXMath.h>
#include "PhysicsHelper.h"
#include "EventTypes.h"
#include "Trace.h"
#include "MoveableUnitComponent.h"
#include "rpc/server.h"
#include <thread>
#include <mutex>
#include <queue>

using namespace DirectX;

struct ObjectData
{
	std::string Mesh;
	std::string Material;
	XMFLOAT3	Scale;
	XMFLOAT3	RotationInAngles;
};

class ObjectInferenceSystem : public ISystem
{
public:
	virtual void Initialize() override
	{
		objectMap = std::unordered_map<std::string_view, ObjectData>{
			{"soccer" , { "Assets/Models/soccer.obj", "soccer", XMFLOAT3(0.5f,0.5f,0.5f)}},
			{"helmet" , { "Assets/Models/helmet.obj", "helmet", XMFLOAT3(0.05f,0.05f,0.05f), XMFLOAT3(90.f, 0.f, 0.f)}},
			{"cat" , { "Assets/Models/cat.obj", "cat", XMFLOAT3(0.05f,0.05f,0.05f), XMFLOAT3(90.f, 0.f, 0.f)}},
			{"basketball" , { "Assets/Models/basketball.obj", "basketball", XMFLOAT3(0.04f,0.04f,0.04f)}}
		};

		serverThread = std::thread([this]()
			{
				server = new rpc::server(8080);
				server->bind("OnObjectInference", [this](std::string object)->bool
					{
						return QueueObjectInstantiation(object);
					});
				server->run();
			});
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		std::vector<std::string> objects;
		{
			std::scoped_lock<std::mutex> lock{ mutex };
			while (!objectQueue.empty())
			{
				objects.push_back(objectQueue.front());
				objectQueue.pop();
			}
		}

		for (auto& object : objects)
		{
			InstantiateObject(object);
		}
	}

	virtual void Destroy() override
	{
		server->stop();
		serverThread.join();
		delete server;
	}
private:
	std::thread serverThread;
	rpc::server* server;
	std::queue<std::string> objectQueue;
	std::mutex mutex;
	std::unordered_map<std::string_view, ObjectData> objectMap;
	std::vector<EntityHandle> createEntities;

	bool QueueObjectInstantiation(std::string objectName)
	{
		es::Log("Queued: '%s'", objectName.c_str());
		{
			std::scoped_lock<std::mutex> lock{ mutex };
			objectQueue.push(objectName);
		}
		return true;
	}

	void InstantiateObject(std::string objectName)
	{
		uint32 count = 0;
		auto& camera = entityManager->GetComponents<CameraComponent>(count)[0].CameraInstance;
		auto objectData = objectMap[objectName];
		auto transform = DefaultTransform;

		transform.Scale = objectData.Scale;
		constexpr float CAngleToRads = XM_PI / 180.f;
		auto quaternion = XMQuaternionRotationRollPitchYaw(objectData.RotationInAngles.x * CAngleToRads, objectData.RotationInAngles.y * CAngleToRads, objectData.RotationInAngles.z * CAngleToRads);
		transform.Rotation.w = 1.f;
		XMStoreFloat4(&transform.Rotation, quaternion);

		auto spawnPos = XMLoadFloat3(&camera.Position) + XMLoadFloat3(&camera.Direction) * 5.f;
		XMStoreFloat3(&transform.Position, spawnPos);

		auto mesh = GContext->MeshManager->GetMeshHandle(objectData.Mesh.c_str());
		auto material = GContext->ShaderResourceManager->GetMaterialHandle(objectData.Material.c_str());

		auto e = entityManager->CreateEntity(transform);
		entityManager->AddComponent<DrawableComponent>(e, DrawableComponent::Create(mesh, material));
		es::Log("Instantiated: '%s'", objectName.c_str());
		createEntities.push_back(e);
	}
};