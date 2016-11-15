#include "Scene.h"
#include <DirectXMath.h>

using namespace DirectX;

Scene::Scene(uint32_t width, uint32_t height, Input & input) : _width(width), _height(height), _input(input), _camera(90.0f, (float)_width / (float)_height, 0.01f, 100.0f, XMFLOAT3(3.0f, 3.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f))
{
	srand(1337U);
	memset(&_numObjects, 0, sizeof(SceneData::CountData));
	/*_AddSphere(XMFLOAT3(0.0f, 0.0f, 3.0f), 2.0f, XMFLOAT3(1.0f, 0.0f, 0.0f));
	_AddSphere(XMFLOAT3(5.0f, 0.5f, 3.0f), 0.5f, XMFLOAT3(0.0f, 1.0f, 0.0f));*/


	//_AddTriangle(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 1.0f), XMFLOAT3(0.0f, -1.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 0.0f));

	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	for(int i = 0;i  <200; i++)
		_AddRandomSphere();
	_AddPointLight(XMFLOAT3(100.0f, 0.0f, 3.0f), 1.0f);
	//_AddPointLight(XMFLOAT3(10.0f, 0.0f, 3.0f), 1.5f);
}

Scene::~Scene()
{
}

uint8_t Scene::Update(float deltaTime)
{
	if (_input.IsKeyPushed(Input::Keys::Escape))
		PostQuitMessage(0);

	float step = 10.0f * deltaTime;
	float msen = 1.0f;
	if (_input.IsKeyDown(Input::Keys::W))
		_camera.MoveForward(step);
	if (_input.IsKeyDown(Input::Keys::S))
		_camera.MoveForward(-step);

	if (_input.IsKeyDown(Input::Keys::D))
		_camera.MoveRight(step);
	if (_input.IsKeyDown(Input::Keys::A))
		_camera.MoveRight(-step);

	if (_input.IsKeyDown(Input::Keys::LeftShift))
		_camera.MoveUp(step);
	if (_input.IsKeyDown(Input::Keys::LeftControl))
		_camera.MoveUp(-step);

	int32_t xd, yd;
	_input.GetMouseDiff(xd, yd);
	if (xd)
		_camera.RotateYaw(-xd*deltaTime*msen);
	if (yd)
		_camera.RotatePitch(-yd*deltaTime*msen);

	return 0;
}

const SceneData::CountData& Scene::GetCounts() const
{
	return _numObjects;
}

const SceneData::Sphere& Scene::GetSpheres() const
{
	return _spheres;
}

const SceneData::Triangle& Scene::GetTriangles() const
{
	return _triangles;
}

const SceneData::PointLight& Scene::GetPointLights() const
{
	return _pointLights;
}

Camera * Scene::GetCamera()
{
	return &_camera;
}

const void Scene::_AddSphere(const DirectX::XMFLOAT3 & pos, float radius, const DirectX::XMFLOAT3 & color)
{
	if (_numObjects.numSpheres < SceneData::maxSpheres)
	{
		_spheres.Position3_Radius_1[_numObjects.numSpheres] = XMFLOAT4(pos.x, pos.y, pos.z, radius);
		_spheres.Color[_numObjects.numSpheres] = XMFLOAT4(color.x, color.y, color.z, 1.0f);
		_numObjects.numSpheres++;
	}
}

const void Scene::_AddTriangle(const DirectX::XMFLOAT3 & p0, const DirectX::XMFLOAT3 & p1, const DirectX::XMFLOAT3 & p2, const DirectX::XMFLOAT3 & color)
{
	if (_numObjects.numTriangles < SceneData::maxTriangles)
	{
		_triangles.p0[_numObjects.numTriangles] = XMFLOAT4(p0.x, p0.y, p0.z, 1.0f);
		_triangles.p1[_numObjects.numTriangles] = XMFLOAT4(p1.x, p1.y, p1.z, 1.0f);
		_triangles.p2[_numObjects.numTriangles] = XMFLOAT4(p2.x, p2.y, p2.z, 1.0f);
		_triangles.Color[_numObjects.numTriangles] = XMFLOAT4(color.x, color.y, color.z, 1.0f);
		_numObjects.numTriangles++;
	}
}

const void Scene::_AddRandomSphere()
{
	XMFLOAT3 pos = XMFLOAT3((rand() % 10000 - 5000) / 100.0f, (rand() % 10000 - 5000) / 100.0f, (rand() % 10000 - 5000) / 100.0f);
	XMFLOAT3 Color = XMFLOAT3((rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f);
	float radius = (rand() % 1000 - 500) / 200.0f;
	_AddSphere(pos, radius, Color);
}

const void Scene::_AddPointLight(const DirectX::XMFLOAT3 & pos, float luminosity)
{
	if (_numObjects.numPointLights < SceneData::maxPointLights)
	{
		_pointLights.Position3_Luminosity1[_numObjects.numPointLights] = XMFLOAT4(pos.x, pos.y, pos.z, luminosity);
		_numObjects.numPointLights++;
	}
}
