#ifndef _SCENE_H_
#define _SCENE_H_

#pragma once
#include "ComputeShaderDataStructs.h"
#include "Input.h"
#include "Camera.h"


class Scene
{
public:
	static const uint32_t maxSpheres = 1024;
	static const uint32_t maxPointLights = 1024;

	Scene(uint32_t width, uint32_t height, Input& input);
	~Scene();

	bool Update(float deltaTime);

	const CountData* GetCounts()const;
	const Sphere* GetSpheres()const;
	const PointLight* GetPointLights()const;

	Camera* GetCamera();
private:
	const void _AddSphere(const DirectX::XMFLOAT3& pos, float radius, const DirectX::XMFLOAT3& color);
	const void _AddRandomSphere();
	const void _AddPointLight(const DirectX::XMFLOAT3& pos, float luminosity);

	uint32_t _width, _height;

	Input& _input;
	Camera _camera;

	

	Sphere _spheres[maxSpheres];
	PointLight _pointLights[maxPointLights];

	CountData _numObjects;

};

#endif