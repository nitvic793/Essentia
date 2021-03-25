#pragma once

#include "EventSystem.h"

struct GameStartEvent : public es::IEvent
{
	float totalTime;
};

struct SelectEntityEvent : public es::IEvent
{
	EntityHandle entity;
};

struct TransformUpdateEvent : public es::IEvent
{
	EntityHandle entity;
};

template<typename T>
struct ComponentUpdateEvent : public es::IEvent
{
	T* component;
	EntityHandle entity;
};

struct IComponentUpdateEvent : public es::IEvent
{
	ComponentData	componentData;
	EntityHandle	entity;
};

template<typename T>
struct ComponentAddEvent : public es::IEvent
{
	T* component;
	EntityHandle entity;
};


struct ReloadScriptSystemEvent : public es::IEvent
{
	float totalTime;
};