
#ifndef _COMPUTE_SHADER_DATA_STRUCTS_H_
#define _COMPUTE_SHADER_DATA_STRUCTS_H_
#pragma once
#include <DirectXMath.h>
#include <stdint.h>

namespace SceneData
{
	static const uint32_t maxSpheres = 512;
	static const uint32_t maxTriangles = 512;
	static const uint32_t maxPointLights = 512;

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
		uint32_t numTexTriangles;
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

	static const uint32_t texturedTriangleSize = sizeof(DirectX::XMFLOAT4) * 3 + sizeof(DirectX::XMFLOAT2) * 3;
	struct TexturedTriangle
	{
		DirectX::XMFLOAT4* p0_textureID = nullptr;
		DirectX::XMFLOAT4* p1 = nullptr;
		DirectX::XMFLOAT4* p2 = nullptr;
		DirectX::XMFLOAT2* t0 = nullptr;
		DirectX::XMFLOAT2* t1 = nullptr;
		DirectX::XMFLOAT2* t2 = nullptr;
	};

	static const uint32_t pointLightSize = sizeof(DirectX::XMFLOAT4);
	struct PointLight
	{
		DirectX::XMFLOAT4 Position3_Luminosity1[maxPointLights];
	};
}
#endif