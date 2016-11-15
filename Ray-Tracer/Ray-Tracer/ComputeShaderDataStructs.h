
#ifndef _COMPUTE_SHADER_DATA_STRUCTS_H_
#define _COMPUTE_SHADER_DATA_STRUCTS_H_
#pragma once
#include <DirectXMath.h>
#include <stdint.h>

namespace SceneData
{
	static const uint32_t maxSpheres = 1024;
	static const uint32_t maxTriangles = 1024;
	static const uint32_t maxPointLights = 1024;

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
		uint32_t numTriangles;
		uint32_t numPointLights;
	};

	static const uint32_t sphereSize = sizeof(DirectX::XMFLOAT4) * 2;
	struct Sphere
	{
		DirectX::XMFLOAT4 Position3_Radius_1[maxSpheres];
		DirectX::XMFLOAT4 Color[maxSpheres];
	};

	static const uint32_t triangleSize = sizeof(DirectX::XMFLOAT4) * 4;
	struct Triangle
	{
		DirectX::XMFLOAT4 p0[maxTriangles];
		DirectX::XMFLOAT4 p1[maxTriangles];
		DirectX::XMFLOAT4 p2[maxTriangles];
		DirectX::XMFLOAT4 Color[maxTriangles];
	};

	static const uint32_t pointLightSize = sizeof(DirectX::XMFLOAT4);
	struct PointLight
	{
		DirectX::XMFLOAT4 Position3_Luminosity1[maxPointLights];
	};
}
#endif