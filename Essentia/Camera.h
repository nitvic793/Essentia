#pragma once

#include <DirectXMath.h>

const DirectX::XMFLOAT3 DefaultUp(0.f, 1.f, 0.f);

struct Camera
{
public:
	Camera(float width, float height, float nearZ = 0.1f, float farZ = 100.f, float fovInAngles = 45.f);
	void						Update(float deltaTime = 0.f, float totalTime = 0.f);
	void						UpdateView();
	void						UpdateProjection(float width, float height, float nearZ = 0.1f, float farZ = 100.f, float fovInAngles = 45.f);
	const DirectX::XMFLOAT4X4&	GetView();
	const DirectX::XMFLOAT4X4&	GetProjection();
	DirectX::XMFLOAT4X4			GetViewTransposed();
	DirectX::XMFLOAT4X4			GetProjectionTransposed();

	DirectX::XMFLOAT3			Position;
	DirectX::XMFLOAT3			Direction;

	DirectX::XMFLOAT4X4			Projection;
	DirectX::XMFLOAT4X4			View;

private:
	DirectX::XMMATRIX XM_CALLCONV GetViewMatrix();
};