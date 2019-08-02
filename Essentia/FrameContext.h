#pragma once

#include "Camera.h"
#include "BaseComponents.h"

class Timer;

struct FrameContext
{
	Camera* Camera;
	Timer* timer;
	std::vector<DirectX::XMFLOAT4X4>	WorldMatrices;
	DrawableComponent* Drawables;
	uint32								DrawableCount;
};