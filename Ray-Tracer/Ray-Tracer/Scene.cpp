#include "Scene.h"
#include <DirectXMath.h>
#include <fstream>
using namespace std;
using namespace DirectX;

Scene::Scene(uint32_t width, uint32_t height, Input & input) : _width(width), _height(height), _input(input), _camera(90.0f, (float)_width / (float)_height, 0.01f, 100.0f, XMFLOAT3(3.0f, 3.0f, -10.0f), XMFLOAT3(0.0f, 0.0f, 0.0f))
{
	srand(1337U);
	memset(&_numObjects, 0, sizeof(SceneData::CountData));
	_AddSphere(XMFLOAT3(0.0f, 0.0f, 0.0f), 2.0f, XMFLOAT3(1.0f, 0.0f, 0.0f));
	_AddSphere(XMFLOAT3(5.0f, 0.5f, 0.0f), 0.5f, XMFLOAT3(0.0f, 1.0f, 0.0f));


	_AddTriangle(XMFLOAT3(1.5f, 0.0f, 0.0f), XMFLOAT3(1.5f, 3.0f, 4.0f), XMFLOAT3(3.0f, -2.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f));

	// Bottom
	_AddTriangle(XMFLOAT3(-10.0f, -10.0f, -10.0f), XMFLOAT3(-10.0f, -10.0f, 10.0f), XMFLOAT3(10.0f, -10.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	_AddTriangle(XMFLOAT3(-10.0f, -10.0f, -10.0f), XMFLOAT3(10.0f, -10.0f, 10.0f), XMFLOAT3(10.0f, -10.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	// Top
	_AddTriangle(XMFLOAT3(-10.0f, 10.0f, -10.0f), XMFLOAT3(10.0f, 10.0f, -10.0f), XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	_AddTriangle(XMFLOAT3(-10.0f, 10.0f, -10.0f), XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(-10.0f, 10.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	// Left
	_AddTriangle(XMFLOAT3(-10.0f, -10.0f, -10.0f), XMFLOAT3(-10.0f, 10.0f, -10.0f), XMFLOAT3(-10.0f, 10.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	_AddTriangle(XMFLOAT3(-10.0f, -10.0f, -10.0f), XMFLOAT3(-10.0f, 10.0f, 10.0f), XMFLOAT3(-10.0f, -10.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));


	// Right
	_AddTriangle(XMFLOAT3(10.0f, -10.0f, -10.0f), XMFLOAT3(10.0f, -10.0f, 10.0f), XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	_AddTriangle(XMFLOAT3(10.0f, -10.0f, -10.0f), XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(10.0f, 10.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	// Back
	_AddTriangle(XMFLOAT3(-10.0f, -10.0f, 10.0f), XMFLOAT3(-10.0f, 10.0f, 10.0f), XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	_AddTriangle(XMFLOAT3(-10.0f, -10.0f, 10.0f), XMFLOAT3(10.0f, 10.0f, 10.0f), XMFLOAT3(10.0f, -10.0f, 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	// Front
	_AddTriangle(XMFLOAT3(-10.0f, -10.0f, -10.0f), XMFLOAT3(10.0f, -10.0f, -10.0f), XMFLOAT3(10.0f, 10.0f,- 10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));
	_AddTriangle(XMFLOAT3(-10.0f, -10.0f, -10.0f), XMFLOAT3(10.0f, 10.0f, -10.0f), XMFLOAT3(-10.0f, 10.0f, -10.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	//_AddRandomSphere();
	for(int i = 0;i  <10; i++)
		_AddRandomSphere();
	_AddPointLight(XMFLOAT3(-5.0f, 7.0, -5.0f), 0.5f);

	//_AddSphere(XMFLOAT3(10.0f, 0.0f, 3.0f), 0.5f, XMFLOAT3(0.0f, 1.0f, 0.0f));
	_AddPointLight(XMFLOAT3(8.0f, 0.0f, 3.0f), 0.5f);

	std::vector<const char*> files;
	vector<std::pair<ArfData::Data, ArfData::DataPointers>> data;
	vector<XMMATRIX> mats;
	mats.push_back(XMMatrixIdentity());
	files.push_back("Meshes/Cube.arf");

	_LoadMeshes(files, data);

	_Interleave(data, mats);




	for (auto& d : data)
		operator delete(d.second.buffer);


}

Scene::~Scene()
{

	delete[] _textureTriangles.p0_textureID;
	delete[] _textureTriangles.p1;
	delete[] _textureTriangles.p2;
	delete[] _textureTriangles.t0;
	delete[] _textureTriangles.t1;
	delete[] _textureTriangles.t2;
}

uint8_t Scene::Update(float deltaTime)
{
	if (_input.IsKeyPushed(Input::Keys::Escape))
		PostQuitMessage(0);

	float step = 10.0f * deltaTime;
	float msen = 0.001f;
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
		_camera.RotateYaw(-xd*msen);
	if (yd)
		_camera.RotatePitch(-yd*msen);


	_Rotate(_spheres.Position3_Radius_1[1], deltaTime*0.5);
	_Rotate(_pointLights.Position3_Luminosity1[0], deltaTime*0.3f);
	return 1;
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

const SceneData::TexturedTriangle & Scene::GetTexturedTriangles() const
{
	return _textureTriangles;
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
	XMFLOAT3 pos = XMFLOAT3((rand() % 200 - 100) / 10.0f, (rand() % 200 - 100) / 10.0f, (rand() % 200 - 100) / 10.0f);
	XMFLOAT3 Color = XMFLOAT3((rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f, (rand() % 10000) / 10000.0f);
	float radius = (rand() % 8 - 4) / 4.0f;
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

const void Scene::_Rotate(DirectX::XMFLOAT4 & pos, float amount)
{
	XMMATRIX rotMatrix = XMMatrixRotationY(amount);
	XMFLOAT4 fpos = pos;

	XMVECTOR vpos = XMLoadFloat4(&fpos);
	vpos = XMVector3TransformCoord(vpos, rotMatrix);
	XMStoreFloat4(&pos, vpos);
	pos.w = fpos.w;
}


void Scene::_Interleave(std::vector<std::pair<ArfData::Data, ArfData::DataPointers>>& data, vector<XMMATRIX>& transforms)
{
	_numObjects.numTexTriangles = 0;
	for (auto& d : data)
	{
		_numObjects.numTexTriangles += d.first.NumFace;
	}

	_textureTriangles.p0_textureID = new XMFLOAT4[_numObjects.numTexTriangles];
	_textureTriangles.p1 = new XMFLOAT4[_numObjects.numTexTriangles];
	_textureTriangles.p2 = new XMFLOAT4[_numObjects.numTexTriangles];
	_textureTriangles.t0 = new XMFLOAT2[_numObjects.numTexTriangles];
	_textureTriangles.t1 = new XMFLOAT2[_numObjects.numTexTriangles];
	_textureTriangles.t2 = new XMFLOAT2[_numObjects.numTexTriangles];

	// Interleave data
	uint32_t index = 0;
	for (uint32_t ID = 0; ID < data.size(); ID++)
	{
		auto& d = data[ID];
		XMMATRIX& mat = transforms[ID];

		for (uint32_t i = 0; i < d.first.NumSubMesh; i++)
		{
			for (uint32_t j = d.second.subMesh[i].faceStart; j < d.second.subMesh[i].faceCount; j++)
			{
				auto& face = d.second.faces[j];

				XMVECTOR p0 = XMLoadFloat3((XMFLOAT3*)&d.second.positions[face.indices[0].index[0] - 1]);
				XMVECTOR p1 = XMLoadFloat3((XMFLOAT3*)&d.second.positions[face.indices[1].index[0] - 1]);
				XMVECTOR p2 = XMLoadFloat3((XMFLOAT3*)&d.second.positions[face.indices[2].index[0] - 1]);

				p0 = XMVector3TransformCoord(p0, mat);
				p1 = XMVector3TransformCoord(p1, mat);
				p2 = XMVector3TransformCoord(p2, mat);

				XMStoreFloat4(&_textureTriangles.p0_textureID[index], p0);
				_textureTriangles.p0_textureID[index].w = ID;
				XMStoreFloat4(&_textureTriangles.p1[index], p0);
				XMStoreFloat4(&_textureTriangles.p2[index], p2);

				memcpy(&_textureTriangles.t0[index], &d.second.texCoords[face.indices[0].index[1] - 1], sizeof(XMFLOAT2));
				memcpy(&_textureTriangles.t1[index], &d.second.texCoords[face.indices[1].index[1] - 1], sizeof(XMFLOAT2));
				memcpy(&_textureTriangles.t2[index], &d.second.texCoords[face.indices[2].index[1] - 1], sizeof(XMFLOAT2));


				index++;

			}
		}



	}

}

void Scene::_LoadMeshes(const std::vector<const char*>& files, vector<std::pair<ArfData::Data, ArfData::DataPointers>>& data)
{
	for (auto& f : files)
	{
		data.push_back({ ArfData::Data(), ArfData::DataPointers() });
		_LoadMesh(f, data[data.size() - 1]);
	}
	

}

void Scene::_LoadMesh(const char* filename, std::pair<ArfData::Data, ArfData::DataPointers>& data)
{
	ifstream file(filename, ios::binary);
	if (!file.is_open())
		throw filename;

	file.read((char*)&data.first, sizeof(ArfData::Data));
	data.second.buffer = operator new(data.first.allocated);
	file.read((char*)data.second.buffer, data.first.allocated);
	data.second.positions = (ArfData::Position*)(data.second.buffer);
	data.second.texCoords = (ArfData::TexCoord*)(data.second.positions + data.first.NumPos);
	data.second.normals = (ArfData::Normal*)(data.second.texCoords + data.first.NumTex);
	data.second.faces = (ArfData::Face*)(data.second.normals + data.first.NumNorm);
	data.second.subMesh = (ArfData::SubMesh*)(data.second.faces + data.first.NumFace);

	file.close();
}
