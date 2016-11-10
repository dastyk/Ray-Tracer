#include "Camera.h"
#include "stdafx.h"
using namespace DirectX;


Camera::Camera(float fov, float aspect, float nearp, float farp, const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot)
	: _fov(fov),
	_aspect(aspect),
	_nearp(nearp),
	_farp(farp),
	_position(pos),
	_rotation(rot)
{
	XMMATRIX projM = DirectX::XMMatrixPerspectiveFovLH(_fov, _aspect, _nearp, _farp);
	DirectX::XMStoreFloat4x4(&_projectionMatrix, projM);

	_Rotate();
}


Camera::~Camera()
{
}

const void Camera::MoveForward(const float amount)
{
	XMVECTOR dir = XMLoadFloat3(&_forward);
	XMVECTOR pos = XMLoadFloat3(&_position);

	pos += dir*amount;

	XMStoreFloat3(&_position, pos);

	_Transform();
}

const void Camera::MoveRight(const float amount)
{
	XMVECTOR dir = XMLoadFloat3(&_right);
	XMVECTOR pos = XMLoadFloat3(&_position);

	pos += dir*amount;
	XMStoreFloat3(&_position, pos);
	_Transform();
}

const void Camera::MoveUp(const float amount)
{
	XMVECTOR dir = XMLoadFloat3(&_up);
	XMVECTOR pos = XMLoadFloat3(&_position);

	pos += dir*amount;
	XMStoreFloat3(&_position, pos);
	_Transform();
}

const void Camera::RotateYaw(const float radians)
{
	_rotation.y += radians;
	_rotation.y = (_rotation.y > 2.0f * PI) ? 0.0f : _rotation.y;
	_rotation.y = (_rotation.y < 0.0f) ? 2.0f * PI : _rotation.y;

	_Rotate();
}

const void Camera::RotatePitch(const float radians)
{
	_rotation.x += radians;
	_rotation.x = (_rotation.x > 2.0f * PI) ? 0.0f : _rotation.x;
	_rotation.x = (_rotation.x < 0.0f) ? 2.0f * PI : _rotation.x;

	_Rotate();
}

const void Camera::RotateRoll(const float radians)
{
	_rotation.z += radians;
	_rotation.z = (_rotation.z > 2.0f * PI) ? 0.0f : _rotation.z;
	_rotation.z = (_rotation.z < 0.0f) ? 2.0f * PI : _rotation.z;

	_Rotate();
}

const DirectX::XMFLOAT3 & Camera::GetPosition() const
{
	return _position;
}

const DirectX::XMFLOAT3 & Camera::GetForward() const
{
	return _forward;
}

const DirectX::XMFLOAT4X4 & Camera::GetViewProjInv() const
{
	return _viewProjectionInvMatrix;
}

const DirectX::XMFLOAT4X4 & Camera::GetViewInv() const
{
	return _viewInvMatrix;
}

const float Camera::GetAspect() const
{
	return _aspect;
}

const float Camera::GetFov() const
{
	return _fov;
}

const float Camera::GetNearP() const
{
	return _nearp;
}

const float Camera::GetFarP() const
{
	return _farp;
}


const void Camera::_Rotate()
{
	XMVECTOR forward, up, right;
	forward = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	right = XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);

	XMVECTOR lookAt;
	XMMATRIX rotationMatrix;

	rotationMatrix = XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&_rotation));


	forward = XMVector3Transform(forward, rotationMatrix);
	up = XMVector3Transform(up, rotationMatrix);
	right = XMVector3Transform(right, rotationMatrix);

	XMStoreFloat3(&_forward, forward);
	XMStoreFloat3(&_up, up);
	XMStoreFloat3(&_right, right);

	_Transform();
}

const void Camera::_Transform()
{
	XMVECTOR pos, forward, up, right;
	pos = XMLoadFloat3(&_position);
	forward = XMLoadFloat3(&_forward);
	up = XMLoadFloat3(&_up);

	XMVECTOR lookAt = XMVectorAdd(pos, forward);

	XMMATRIX viewMatrix = XMMatrixLookAtLH(pos, lookAt, up);
	DirectX::XMStoreFloat4x4(&_viewMatrix, viewMatrix);
	XMMATRIX viewMatrixInv = XMMatrixInverse(nullptr, viewMatrix);
	DirectX::XMStoreFloat4x4(&_viewInvMatrix, viewMatrixInv);


	XMMATRIX viewProj = viewMatrix * DirectX::XMLoadFloat4x4(&_projectionMatrix);
	DirectX::XMStoreFloat4x4(&_viewProjectionMatrix, viewProj);
	XMMATRIX viewprojInv = XMMatrixInverse(nullptr, viewProj);
	DirectX::XMStoreFloat4x4(&_viewProjectionInvMatrix, viewprojInv);

	float x = ((2.0f * (float)400) / 800.0f) - 1.0f;
	float y = (((2.0f * (float)400) / 800.0f) - 1.0f) * -1.0f;
	XMVECTOR tp = XMVectorSet(x, y, 0.0f, 1.0f);
	XMVECTOR tp2 = XMVector3TransformCoord(tp, viewprojInv);
	XMVECTOR dir = tp2 - pos;

	//_position.x = 10.0f;
	return void();
}
