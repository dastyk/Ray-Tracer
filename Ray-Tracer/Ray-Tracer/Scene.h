#ifndef _SCENE_H_
#define _SCENE_H_

#pragma once
#include "ComputeShaderDataStructs.h"
#include "Input.h"
#include "Camera.h"
#include <ArfData.h>
#include <vector>
#include <Parsers.h>
class Scene
{
public:
	Scene(uint32_t width, uint32_t height, Input& input);
	~Scene();

	uint8_t Update(float deltaTime);

	const SceneData::CountData& GetCounts()const;
	const SceneData::Sphere& GetSpheres()const;
	const SceneData::Triangle& GetTriangles()const;
	const SceneData::PointLight& GetPointLights()const;
	const SceneData::SpotLights& GetSpotLights()const;
	const SceneData::TexturedTriangle& GetTexturedTriangles()const;

	const void UpdateSphere(uint32_t ID);
	const void UpdateTriangle(uint32_t ID);
	const void UpdateTexTriangle(const DirectX::XMFLOAT3& pos);
	const DirectX::XMFLOAT4X4* GetTranslations(uint32_t& numMesh);
	Camera* GetCamera();
private:
	const void _AddSphere(const DirectX::XMFLOAT3& pos, float radius, const DirectX::XMFLOAT3& color);
	const void _AddTriangle(const DirectX::XMFLOAT3& p0, const DirectX::XMFLOAT3& p1, const DirectX::XMFLOAT3& p2, const DirectX::XMFLOAT3& color);
	const void _AddRandomSphere();
	const void _AddRandomPointLight();
	const void _AddPointLight(const DirectX::XMFLOAT3& pos, float luminosity);
	const void _AddSpotLight(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& dir, float range, float theta, float phi, float luminosity);
	const void _Rotate(DirectX::XMFLOAT4& pos, float amount);


	uint32_t _width, _height;

	Input& _input;
	Camera _camera;

	

	SceneData::Sphere _spheres;
	SceneData::Triangle _triangles;
	SceneData::PointLight _pointLights;
	SceneData::TexturedTriangle _textureTriangles;
	SceneData::SpotLights _spotLights;
	SceneData::CountData _numObjects;

	DirectX::XMFLOAT4X4* _translations;
	uint32_t _numMesh;


	void _Interleave(std::vector<std::pair<ArfData::Data, ArfData::DataPointers>>& data, const std::vector<uint32_t>& textureIDs);
	void _LoadMeshes(const std::vector<const char*>& files, std::vector<std::pair<ArfData::Data, ArfData::DataPointers>>& data);
	void _LoadMesh(const char* filename, std::pair<ArfData::Data, ArfData::DataPointers>& data);
};

#endif