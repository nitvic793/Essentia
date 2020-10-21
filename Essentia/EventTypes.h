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