#pragma once

#include "Camera.h"
#include "RenderComponents.h"
#include "Utility.h"

class Timer;

struct FrameContext
{
	Camera*								Camera;
	Timer*								Timer;
	Vector<DirectX::XMFLOAT4X4>			WorldMatrices;
	Vector<DirectX::XMFLOAT4X4>			ModelWorldMatrices;
	DrawableComponent*					Drawables;
	DrawableModelComponent*				DrawableModels;
	uint32								DrawableCount;
	uint32								DrawableModelCount;
	EntityManager*						EntityManager;
};