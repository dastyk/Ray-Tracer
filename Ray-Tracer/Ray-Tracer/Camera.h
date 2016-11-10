#ifndef _CAMERA_H_
#define _CAMERA_H_

#pragma once
#include <DirectXMath.h>

class Camera
{
public:
	Camera(float fov, float aspect, float nearp, float farp,const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& rot);
	~Camera();

	const void MoveForward(const float amount);//
	const void MoveRight(const float amount);//
	const void MoveUp(const float amount);//
	const void RotateYaw(const float radians);//
	const void RotatePitch(const float radians);//
	const void RotateRoll(const float radians);

	const DirectX::XMFLOAT3& GetPosition()const;
	const DirectX::XMFLOAT3& GetForward()const;

	const DirectX::XMFLOAT4X4& GetViewProjInv()const;
	const DirectX::XMFLOAT4X4& GetViewInv()const;
	const float GetAspect()const;
	const float GetFov()const;
	const float GetNearP()const;
	const float GetFarP()const;
private:

	float _fov;
	float _aspect;
	float _nearp;
	float _farp;

	DirectX::XMFLOAT3	_position;
	DirectX::XMFLOAT3	_rotation;

	DirectX::XMFLOAT3	_forward;
	DirectX::XMFLOAT3	_up;
	DirectX::XMFLOAT3	_right;

	DirectX::XMFLOAT4X4 _viewMatrix;
	DirectX::XMFLOAT4X4 _viewInvMatrix;
	DirectX::XMFLOAT4X4 _projectionMatrix;
	DirectX::XMFLOAT4X4 _viewProjectionMatrix;
	DirectX::XMFLOAT4X4 _viewProjectionInvMatrix;

	const void _Rotate();
	/** Transform has changed, so update everything.
	* It is assumed that the indices exist.
	*/
	const void _Transform();
};

#endif