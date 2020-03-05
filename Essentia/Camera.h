#pragma once

#include <DirectXMath.h>
#include <DirectXCollision.h>

const DirectX::XMFLOAT3 DefaultUp(0.f, 1.f, 0.f);

struct Camera
{
public:
	Camera(float width, float height, float nearZ = 0.1f, float farZ = 300.f, float fovInAngles = 45.f);
	void						Update(float deltaTime = 0.f, float totalTime = 0.f);
	void						UpdateView();
	void						UpdateProjection(float width, float height, float nearZ = 0.1f, float farZ = 100.f, float fovInAngles = 45.f);
	const DirectX::XMFLOAT4X4&	GetView() const;
	const DirectX::XMFLOAT4X4&	GetProjection() const;
	DirectX::XMFLOAT4X4			GetInverseProjectionTransposed() const;
	DirectX::XMFLOAT4X4			GetInverseViewTransposed() const;
	DirectX::XMFLOAT4X4			GetViewTransposed() const;
	DirectX::XMFLOAT4X4			GetProjectionTransposed() const;

	DirectX::XMFLOAT3			Position;
	DirectX::XMFLOAT3			Rotation;
	DirectX::XMFLOAT3			Direction;

	DirectX::XMFLOAT4X4			Projection;
	DirectX::XMFLOAT4X4			View;
	DirectX::BoundingFrustum	Frustum;
	float						NearZ;
	float						FarZ;
	float						FieldOfView;
	float						Width;
	float						Height;
	bool						IsOrthographic;

private:
	DirectX::XMMATRIX XM_CALLCONV GetViewMatrix();
};