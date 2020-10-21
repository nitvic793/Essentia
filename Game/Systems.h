#pragma once

#include "System.h"
#include "RenderComponents.h"
#include <DirectXMath.h>
#include "PhysicsHelper.h"
#include "EventTypes.h"
#include "Trace.h"
#include "MoveableUnitComponent.h"

using namespace DirectX;

struct MoveUnitEvent : public es::IEvent
{
	XMFLOAT3 TargetPos;
};

class RotationSystem : public ISystem
{
public:
	virtual void Initialize()
	{
		cManager = entityManager->GetComponentManager();
		//auto e = entityManager->CreateEntity(DefaultTransform, 1);
		//entityManager->AddComponent<DrawableComponent>(e, DrawableComponent::Create({ 0 }, { 0 }));
		//entityManager->AddComponent<TestComponent>(e);
		es::GEventBus->Subscribe(this, &RotationSystem::OnGameStart);
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		uint32 count;
		auto camera = cManager->GetAllComponents<CameraComponent>(count);
		{
			auto entities = cManager->GetEntities<BoundingOrientedBoxComponent, MoveableUnitComponent>();
			float distance;
			SelectEntityEvent selectEvent;
			DirectX::XMFLOAT3 intersection;
			for (auto entity : entities)
			{
				BoundingOrientedBoxComponent* component = cManager->GetComponent<BoundingOrientedBoxComponent>(entity);
				if (es::IsIntersecting(component->BoundingOrientedBox, &camera[0].CameraInstance, mouse.x, mouse.y, distance, intersection)
					&& mouse.leftButton
					&& (totalTime - selectTime) > CSelectCooldownTime)
				{
					selectTime = totalTime;
					selectEvent.entity = entity;
					es::GEventBus->Publish(&selectEvent);
					break;
				}
			}
		}

		{
			auto entities = cManager->GetEntities<BoundingOrientedBoxComponent, TerrainComponent>();
			for (auto entity : entities)
			{
				float distance;
				DirectX::XMFLOAT3 intersection;
				BoundingOrientedBoxComponent* component = cManager->GetComponent<BoundingOrientedBoxComponent>(entity);
				if (es::IsIntersecting(component->BoundingOrientedBox, &camera[0].CameraInstance, mouse.x, mouse.y, distance, intersection)
					&& mouse.leftButton
					&& (totalTime - selectTime) > CSelectCooldownTime)
				{
					MoveUnitEvent event;
					event.TargetPos = intersection;
					es::GEventBus->Publish(&event);
					break;
				}
			}
		}



		{
			auto entities = cManager->GetEntities<PointLightComponent>();
			for (auto e : entities)
			{
				auto transform = GetTransform(e);
				transform.Position->z = 10 * sin(totalTime);
				float factor = abs(cos(totalTime));
				auto component = cManager->GetComponent<PointLightComponent>(e);
				//component->Intensity = 1 + factor * 2.f;
				//component->Range = 2 + factor * 8.f;
			}
		}

	}

	void OnGameStart(GameStartEvent* event)
	{
		es::Log("Game start at %d", event->totalTime);
	}

private:
	ComponentManager* cManager = nullptr;
	float				selectTime = 0;
	const float			CSelectCooldownTime = 0.2f;
};


class MoveObjectSystem : public ISystem
{
public:
	virtual void Initialize() override
	{
		es::GEventBus->Subscribe(this, &MoveObjectSystem::OnSelectEntity);
		es::GEventBus->Subscribe(this, &MoveObjectSystem::OnMoveUnitEvent);
	}

	virtual void Update(float deltaTime, float totalTime) override
	{
		if (enabled)
		{
			auto moveComponent = entityManager->GetComponent<MoveableUnitComponent>(selectedEntity);
			auto posComponent = entityManager->GetComponent<PositionComponent>(selectedEntity);
			XMFLOAT3 position = *posComponent;
			auto pos = XMLoadFloat3(&position);
			moveComponent->TargetPos.y = position.y;
			auto target = XMLoadFloat3(&moveComponent->TargetPos);

			auto direction = XMVector3Normalize(target - pos);
			pos = pos + direction * deltaTime * moveComponent->MoveSpeed;
			XMStoreFloat3(&position, pos);
			*posComponent = position;
			
			auto distance = XMVectorGetX(XMVector3Length(target - pos));
			const float epsilon = 0.1f;
			if (distance <= epsilon)
			{
				enabled = false;
			}
		}
	}

	void OnSelectEntity(SelectEntityEvent* event)
	{
		MoveableUnitComponent* comp = entityManager->GetComponent<MoveableUnitComponent>(event->entity);
		if (comp != nullptr)
		{
			selectedEntity = event->entity;
			isEntitySelected = true;
		}
	}

	void OnMoveUnitEvent(MoveUnitEvent* event)
	{
		if (isEntitySelected)
		{
			auto moveComponent = entityManager->GetComponent<MoveableUnitComponent>(selectedEntity);
			moveComponent->TargetPos = event->TargetPos;
			enabled = true;
		}
	}

private:
	EntityHandle selectedEntity;
	bool enabled = false;
	bool isEntitySelected = false;
};