#pragma once

#include "EventSystem.h"

struct GameStartEvent : public es::IEvent
{
	float totalTime;
};