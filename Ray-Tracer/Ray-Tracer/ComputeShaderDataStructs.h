
#ifndef _COMPUTE_SHADER_DATA_STRUCTS_H_
#define _COMPUTE_SHADER_DATA_STRUCTS_H_
#pragma once
#include <DirectXMath.h>
#include <stdint.h>

struct CameraData
{
	DirectX::XMFLOAT4X4 ViewInv;
	float Aspect;
	float ScreenWidth;
	float ScreenHeight;
	float Fov;
	DirectX::XMFLOAT3 Pos;
	float NearP;
	DirectX::XMFLOAT3 Forward;
	float FarP;
};

struct CountData
{
	uint32_t numSpheres;
	uint32_t numPointLights;
};


struct Sphere
{
	DirectX::XMFLOAT3 Pos;
	float radius;
	DirectX::XMFLOAT3 Color;
};
struct PointLight
{
	DirectX::XMFLOAT3 Pos;
	float luminosity;
};

#endif