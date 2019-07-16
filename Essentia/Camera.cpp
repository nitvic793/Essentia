#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera(float width, float height, float nearZ, float farZ, float fovInAngles)
{
	Position = XMFLOAT3(0, 0, -5);
	Direction = XMFLOAT3(0, 0, 1);
	
	UpdateProjection(width, height, nearZ, farZ, fovInAngles);
	UpdateView();
}

void Camera::Update(float deltaTime, float totalTime)
{
	UpdateView();
}

void Camera::UpdateView()
{
	auto view = GetViewMatrix();
	XMStoreFloat4x4(&View, view);
}

void Camera::UpdateProjection(float width, float height, float nearZ, float farZ, float fovInAngles)
{
	auto projection = XMMatrixPerspectiveFovLH((fovInAngles / 180.f) * XM_PI, width / height, nearZ, farZ);
	XMStoreFloat4x4(&Projection, projection);
}

const XMFLOAT4X4& Camera::GetView()
{
	return View;
}

const DirectX::XMFLOAT4X4& Camera::GetProjection()
{
	return Projection;
}

XMFLOAT4X4 Camera::GetViewTransposed()
{
	XMFLOAT4X4 viewMatrix;
	auto view = XMLoadFloat4x4(&View);
	view = XMMatrixTranspose(view);
	XMStoreFloat4x4(&viewMatrix, view);
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionTransposed()
{
	XMFLOAT4X4 projMatrix;
	auto proj = XMLoadFloat4x4(&Projection);
	proj = XMMatrixTranspose(proj);
	XMStoreFloat4x4(&projMatrix, proj);
	return projMatrix;
}

DirectX::XMMATRIX XM_CALLCONV Camera::GetViewMatrix()
{
	return XMMatrixLookToLH(
		XMLoadFloat3(&Position),
		XMLoadFloat3(&Direction),
		XMLoadFloat3(&DefaultUp)
	);
}
