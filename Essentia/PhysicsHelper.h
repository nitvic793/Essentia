#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>
#include "Camera.h"

namespace es
{
	bool IsIntersecting(DirectX::BoundingOrientedBox boundingBox, Camera* camera, int mouseX, int mouseY, float& distance);
}