#include "pch.h"
#include "PhysicsHelper.h"
#include "EngineContext.h"
#include "Renderer.h"

using namespace DirectX;

bool es::IsIntersecting(DirectX::BoundingOrientedBox boundingBox, Camera* camera, int mouseX, int mouseY, float& distance)
{
	auto screenSize = GContext->RendererInstance->GetScreenSize();
	uint16_t screenWidth = screenSize.Width;
	uint16_t screenHeight = screenSize.Height;
	auto viewMatrix = XMLoadFloat4x4(&camera->View);
	auto projMatrix = XMLoadFloat4x4(&camera->Projection);

	auto orig = XMVector3Unproject(
		XMVectorSet((float)mouseX, (float)mouseY, 0.f, 0.f),
		0,
		0,
		screenWidth,
		screenHeight,
		0,
		1,
		projMatrix,
		viewMatrix,
		XMMatrixIdentity());

	auto dest = XMVector3Unproject(
		XMVectorSet((float)mouseX, (float)mouseY, 1.f, 0.f),
		0,
		0,
		screenWidth,
		screenHeight,
		0,
		1,
		projMatrix,
		viewMatrix,
		XMMatrixIdentity());

	auto direction = dest - orig;
	direction = XMVector3Normalize(direction);
	bool intersecting = boundingBox.Intersects(orig, direction, distance);
	return intersecting;
}
