#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera(float width, float height, float nearZ, float farZ, float fovInAngles) :
	NearZ(nearZ),
	FarZ(farZ),
	FieldOfView(fovInAngles),
	Width(width),
	Height(height),
	IsOrthographic(false)
{
	Position = XMFLOAT3(0, 0, -5);
	Direction = XMFLOAT3(0, 0, 1);
	Rotation = {};
	//UpdateProjection(width, height, nearZ, farZ, fovInAngles);
	//UpdateView();
	BoundingFrustum::CreateFromMatrix(Frustum, XMLoadFloat4x4(&Projection));
}

void Camera::Update(float deltaTime, float totalTime)
{
	NearZ = XMMax(NearZ, 0.001f);
	UpdateView();
	UpdateProjection(Width, Height, NearZ, FarZ, FieldOfView);

	auto view = GetViewMatrix();
	auto projection = GetProjection();
	XMVECTOR determinant;
	BoundingFrustum::CreateFromMatrix(Frustum, XMLoadFloat4x4(&projection));
	Frustum.Transform(Frustum, XMMatrixInverse(&determinant, view));
}

void Camera::UpdateView()
{
	auto rotation = XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&Rotation));
	auto direction = XMVectorSet(0, 0, 1, 0);
	direction = XMVector3Rotate(direction, rotation);
	XMStoreFloat3(&Direction, direction);
	auto view = GetViewMatrix();
	XMStoreFloat4x4(&View, view);
}

void Camera::UpdateProjection(float width, float height, float nearZ, float farZ, float fovInAngles)
{
	auto projection = IsOrthographic ? XMMatrixOrthographicLH(width, height, nearZ, farZ) : XMMatrixPerspectiveFovLH((fovInAngles / 180.f) * XM_PI, width / height, nearZ, farZ);
	XMStoreFloat4x4(&Projection, projection);
}

const XMFLOAT4X4& Camera::GetView() const
{
	return View;
}

const DirectX::XMFLOAT4X4& Camera::GetProjection() const
{
	return Projection;
}

DirectX::XMFLOAT4X4 Camera::GetInverseProjectionTransposed() const
{
	XMFLOAT4X4 invProjMatrix;
	auto proj = XMLoadFloat4x4(&Projection);
	auto invProj = XMMatrixInverse(nullptr, proj);
	XMStoreFloat4x4(&invProjMatrix, XMMatrixTranspose(invProj));
	return invProjMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetInverseViewTransposed() const
{
	XMFLOAT4X4 invViewMatrix;
	auto view = XMLoadFloat4x4(&View);
	auto invView = XMMatrixInverse(nullptr, view);
	XMStoreFloat4x4(&invViewMatrix, XMMatrixTranspose(invView));
	return invViewMatrix;
}

XMFLOAT4X4 Camera::GetViewTransposed() const
{
	XMFLOAT4X4 viewMatrix;
	auto view = XMLoadFloat4x4(&View);
	view = XMMatrixTranspose(view);
	XMStoreFloat4x4(&viewMatrix, view);
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjectionTransposed() const
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
