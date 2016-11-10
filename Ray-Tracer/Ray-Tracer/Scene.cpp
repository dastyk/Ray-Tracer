#include "Scene.h"
#include <DirectXMath.h>

using namespace DirectX;

Scene::Scene(uint32_t width, uint32_t height, Input & input) : _width(width), _height(height), _input(input), _camera(90.0f, (float)_width / (float)_height, 0.01f, 100.0f, XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 0.0f, 0.0f))
{
	srand(1337U);
	memset(&_numObjects, 0, sizeof(CountData));
	_AddSphere(XMFLOAT3(0.0f, 0.0f, 3.0f), 2.0f, XMFLOAT3(1.0f, 0.0f, 0.0f));
	_AddSphere(XMFLOAT3(4.0f, 0.0f, 3.0f), 0.5f, XMFLOAT3(0.0f, 1.0f, 0.0f));

	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();

	_AddPointLight(XMFLOAT3(6.0f, 0.0f, 3.0f), 1.0f);
	//_AddPointLight(XMFLOAT3(3.0f, 10.0f, 3.0f), 1.5f);
}

Scene::~Scene()
{
}

bool Scene::Update(float deltaTime)
{
	if (_input.IsKeyPushed(Input::Keys::Escape))
		PostQuitMessage(0);

	float step = 1.0f * deltaTime;
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

	return false;
}

const CountData * Scene::GetCounts() const
{
	return &_numObjects;
}

const Sphere * Scene::GetSpheres()const
{
	return _spheres;
}

const PointLight * Scene::GetPointLights() const
{
	return _pointLights;
}

Camera * Scene::GetCamera()
{
	return &_camera;
}

const void Scene::_AddSphere(const DirectX::XMFLOAT3 & pos, float radius, const DirectX::XMFLOAT3 & color)
{
	if (_numObjects.numSpheres < maxSpheres)
	{
		_spheres[_numObjects.numSpheres].Pos = pos;
		_spheres[_numObjects.numSpheres].radius = radius;
		_spheres[_numObjects.numSpheres].Color = color;
		_numObjects.numSpheres++;
	}
}

const void Scene::_AddRandomSphere()
{
	XMFLOAT3 pos = XMFLOAT3((rand() % 1000 - 500) / 100.0f, (rand() % 1000 - 500) / 100.0f, (rand() % 1000 - 500) / 100.0f);
	XMFLOAT3 Color = XMFLOAT3((rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f, (rand() % 1000) / 1000.0f);
	float radius = (rand() % 1000 - 500) / 200.0f;
	_AddSphere(pos, radius, Color);
}

const void Scene::_AddPointLight(const DirectX::XMFLOAT3 & pos, float luminosity)
{
	if (_numObjects.numPointLights < maxPointLights)
	{
		_pointLights[_numObjects.numPointLights].Pos = pos;
		_pointLights[_numObjects.numPointLights].luminosity = luminosity;
		_numObjects.numPointLights++;
	}
}
