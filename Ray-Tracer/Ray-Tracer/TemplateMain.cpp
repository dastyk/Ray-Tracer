#define NOMINMAX
#include "stdafx.h"

#include "ComputeHelp.h"
#include "D3D11Timer.h"
#include "Input.h"
#include "Scene.h"
#include <algorithm>

using namespace DirectX;

#if defined( DEBUG ) || defined( _DEBUG )
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif

#include <stdint.h>



//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------
HINSTANCE					g_hInst					= nullptr;
HWND						g_hWnd					= nullptr;

IDXGISwapChain*				g_SwapChain				= nullptr;
ID3D11Device*				g_Device				= nullptr;
ID3D11DeviceContext*		g_DeviceContext			= nullptr;

ID3D11UnorderedAccessView*  g_BackBufferUAV			= nullptr;  // compute output

ComputeWrap*				g_ComputeSys			= nullptr;
ComputeShader*				g_ComputeShader			= nullptr;

D3D11Timer*					g_Timer					= nullptr;
int g_Width, g_Height;

Input*						g_Input					= nullptr;
Scene*						g_Scene					= nullptr;
ID3D11Buffer*				g_csCameraBuffer		= nullptr;
ID3D11Buffer*				g_csCountbuffer			= nullptr;
ComputeBuffer*				g_csSphereBuffer		= nullptr;
ComputeBuffer*				g_csTriangleBuffer		= nullptr;
ComputeBuffer*				g_csPointLightBuffer	= nullptr;
ComputeBuffer*				g_csTexTriangleBuffer	= nullptr;
ComputeTexture*				g_csTexture1			= nullptr;
ComputeTexture*				g_csNormal1				= nullptr;
ID3D11SamplerState*			g_sampler				= nullptr;
ComputeBuffer*				g_csSpotLightBuffer		= nullptr;

ComputeShader*				g_pickingIntShader		= nullptr;
ComputeShader*				g_pickingCmpShader		= nullptr;
ID3D11Buffer*				g_pickIntBuffer			= nullptr;
ID3D11Buffer*				g_pickCompBuffer		= nullptr;
ComputeBuffer*				g_pickBuff[2]			= { nullptr, nullptr };

double rendertime;
double updatetime;

//--------------------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------------------
HRESULT             InitWindow( HINSTANCE hInstance, int nCmdShow );
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT				Render(float deltaTime);
HRESULT				Update(float deltaTime);

char* FeatureLevelToString(D3D_FEATURE_LEVEL featureLevel)
{
	if(featureLevel == D3D_FEATURE_LEVEL_11_0)
		return "11.0";
	if(featureLevel == D3D_FEATURE_LEVEL_10_1)
		return "10.1";
	if(featureLevel == D3D_FEATURE_LEVEL_10_0)
		return "10.0";

	return "Unknown";
}

//--------------------------------------------------------------------------------------
// Create Direct3D device and swap chain
//--------------------------------------------------------------------------------------
HRESULT Init()
{
	HRESULT hr = S_OK;;

	RECT rc;
	GetClientRect(g_hWnd, &rc);
	g_Width = rc.right - rc.left;;
	g_Height = rc.bottom - rc.top;

	UINT createDeviceFlags = 0;
#if defined( DEBUG ) || defined( _DEBUG )
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverType;

	D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = sizeof(driverTypes) / sizeof(driverTypes[0]);

	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 1;
	sd.BufferDesc.Width = g_Width;
	sd.BufferDesc.Height = g_Height;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
	sd.OutputWindow = g_hWnd;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;

	D3D_FEATURE_LEVEL featureLevelsToTry[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	D3D_FEATURE_LEVEL initiatedFeatureLevel;

	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
	{
		driverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDeviceAndSwapChain(
			nullptr,
			driverType,
			nullptr,
			createDeviceFlags,
			featureLevelsToTry,
			ARRAYSIZE(featureLevelsToTry),
			D3D11_SDK_VERSION,
			&sd,
			&g_SwapChain,
			&g_Device,
			&initiatedFeatureLevel,
			&g_DeviceContext);

		if (SUCCEEDED(hr))
		{
			char title[256];
			sprintf_s(
				title,
				sizeof(title),
				"BTH - Direct3D 11.0 Template | Direct3D 11.0 device initiated with Direct3D %s feature level",
				FeatureLevelToString(initiatedFeatureLevel)
			);
			SetWindowTextA(g_hWnd, title);

			break;
		}
	}
	if (FAILED(hr))
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer;
	hr = g_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr))
		return hr;

	// create shader unordered access view on back buffer for compute shader to write into texture
	hr = g_Device->CreateUnorderedAccessView(pBackBuffer, nullptr, &g_BackBufferUAV);

	// Release, not needed anymore.
	pBackBuffer->Release();

	//create helper sys and compute shader instance
	g_ComputeSys = new ComputeWrap(g_Device, g_DeviceContext);
	g_ComputeShader = g_ComputeSys->CreateComputeShader(_T("Shaders/RayTracer.fx"), nullptr, "main", nullptr);
	g_Timer = new D3D11Timer(g_Device, g_DeviceContext);


	g_Scene = new Scene(g_Width, g_Height, *g_Input);


	const SceneData::CountData& cdata = g_Scene->GetCounts();

	// Camera buffer
	g_csCameraBuffer = g_ComputeSys->CreateConstantBuffer(sizeof(SceneData::CameraData), nullptr, true);


	// Buffer for number of objects
	g_csCountbuffer = g_ComputeSys->CreateConstantBuffer(sizeof(SceneData::CountData), (void*)&cdata, true);

	// Object buffers
	g_csSphereBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::RAW_BUFFER, SceneData::sphereSize, SceneData::maxSpheres, true, false, nullptr, true);
	g_csTriangleBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::RAW_BUFFER, SceneData::triangleSize, SceneData::maxTriangles, true, false, nullptr, true);
	g_csPointLightBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::RAW_BUFFER, SceneData::pointLightSize, SceneData::maxPointLights, true, false, nullptr, true);
	g_csTexTriangleBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::RAW_BUFFER, SceneData::texturedTriangleSize, cdata.numTexTriangles, true, false, nullptr, true);
	g_csSpotLightBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::RAW_BUFFER, SceneData::spotLightSize, SceneData::maxSpotLights , true, false, nullptr, true);

	// Copy sphere data to device
	const SceneData::Sphere& sphereData_h = g_Scene->GetSpheres();
	void* sphereData_d = g_csSphereBuffer->Map<void>();

	memcpy(sphereData_d, sphereData_h.Position3_Radius_1, sizeof(XMFLOAT4)*cdata.numSpheres);
	sphereData_d = (XMFLOAT4*)sphereData_d + SceneData::maxSpheres;

	memcpy(sphereData_d, sphereData_h.Color, sizeof(XMFLOAT4)*cdata.numSpheres);

	g_csSphereBuffer->Unmap();


	// Copy triangle data to device
	const SceneData::Triangle& triangleData_h = g_Scene->GetTriangles();

	void* triangleData_d = g_csTriangleBuffer->Map<void>();

	memcpy(triangleData_d, triangleData_h.p0, sizeof(XMFLOAT4)*cdata.numTriangles);
	triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

	memcpy(triangleData_d, triangleData_h.p1, sizeof(XMFLOAT4)*cdata.numTriangles);
	triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

	memcpy(triangleData_d, triangleData_h.p2, sizeof(XMFLOAT4)*cdata.numTriangles);
	triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

	memcpy(triangleData_d, triangleData_h.Color, sizeof(XMFLOAT4)*cdata.numTriangles);

	g_csTriangleBuffer->Unmap();

	// Copy pointlight data to device
	const SceneData::PointLight& pointLightData_h = g_Scene->GetPointLights();
	void* pointLightData_d = g_csPointLightBuffer->Map<void>();

	memcpy(pointLightData_d, pointLightData_h.Position3_Luminosity1, sizeof(XMFLOAT4)*cdata.numPointLights);

	g_csPointLightBuffer->Unmap();


	// copy the textured triangle data to device
	const SceneData::TexturedTriangle& texTriangleData_h = g_Scene->GetTexturedTriangles();
	void* texTriData_d = g_csTexTriangleBuffer->Map<void>();

	memcpy(texTriData_d, texTriangleData_h.p0_textureID, sizeof(XMFLOAT4)*cdata.numTexTriangles);
	texTriData_d = (XMFLOAT4*)texTriData_d + cdata.numTexTriangles;
	memcpy(texTriData_d, texTriangleData_h.p1, sizeof(XMFLOAT4)*cdata.numTexTriangles);
	texTriData_d = (XMFLOAT4*)texTriData_d + cdata.numTexTriangles;
	memcpy(texTriData_d, texTriangleData_h.p2, sizeof(XMFLOAT4)*cdata.numTexTriangles);
	texTriData_d = (XMFLOAT4*)texTriData_d + cdata.numTexTriangles;

	memcpy(texTriData_d, texTriangleData_h.t0, sizeof(XMFLOAT2)*cdata.numTexTriangles);
	texTriData_d = (XMFLOAT2*)texTriData_d + cdata.numTexTriangles;
	memcpy(texTriData_d, texTriangleData_h.t1, sizeof(XMFLOAT2)*cdata.numTexTriangles);
	texTriData_d = (XMFLOAT2*)texTriData_d + cdata.numTexTriangles;
	memcpy(texTriData_d, texTriangleData_h.t2, sizeof(XMFLOAT2)*cdata.numTexTriangles);
	texTriData_d = (XMFLOAT2*)texTriData_d + cdata.numTexTriangles;

	g_csTexTriangleBuffer->Unmap();


	// Copy spotlight data
	const SceneData::SpotLights& spotLightData_h = g_Scene->GetSpotLights();
	void* spotLightData_d = g_csSpotLightBuffer->Map<void>();

	memcpy(spotLightData_d, spotLightData_h.Position3_Luminosity1, sizeof(XMFLOAT4)*cdata.numSpotLights);
	spotLightData_d = (XMFLOAT4*)spotLightData_d + SceneData::maxSpotLights;
	memcpy(spotLightData_d, spotLightData_h.Direction3_Range1, sizeof(XMFLOAT4)*cdata.numSpotLights);
	spotLightData_d = (XMFLOAT4*)spotLightData_d + SceneData::maxSpotLights;
	memcpy(spotLightData_d, spotLightData_h.Angles, sizeof(XMFLOAT2)*cdata.numSpotLights);

	g_csSpotLightBuffer->Unmap();


	//g_csTexture1 = g_ComputeSys->CreateTexture(L"Textures/Floor_Dif.png");

	std::vector<const wchar_t*> textures;
	textures.push_back(L"Textures/Floor_Dif1.png");
	textures.push_back(L"Textures/Wall_Dif1.png");
	textures.push_back(L"Textures/Wall_DifG1.png");
	
	g_csTexture1 = g_ComputeSys->CreateTextureArray(textures);

	std::vector<const wchar_t*> normals;
	normals.push_back(L"Textures/Floor_NM1.png");
	normals.push_back(L"Textures/Wall_NM1.png");
	normals.push_back(L"Textures/Wall_NMG1.png");
	g_csNormal1 = g_ComputeSys->CreateTextureArray(normals);

	//g_csNormal1 = g_ComputeSys->CreateTexture(L"Textures/Floor_NM.png");



	D3D11_SAMPLER_DESC samd;
	ZeroMemory(&sd, sizeof(sd));
	samd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samd.MaxAnisotropy = 16;
	samd.Filter = D3D11_FILTER_ANISOTROPIC;
	samd.MinLOD = -FLT_MAX;
	samd.MaxLOD = FLT_MAX;
	samd.MipLODBias = 0.0f;
	g_Device->CreateSamplerState(&samd, &g_sampler);
	g_DeviceContext->CSSetSamplers(0, 1, &g_sampler);


	ID3D11ShaderResourceView* srvs[] = 
	{ 
		g_csSphereBuffer->GetResourceView(),
		g_csTriangleBuffer->GetResourceView(),
		g_csPointLightBuffer->GetResourceView(),
		g_csTexTriangleBuffer->GetResourceView(),
		g_csSpotLightBuffer->GetResourceView(),
		nullptr,
		g_csTexture1->GetResourceView(),
		g_csNormal1->GetResourceView()
	};

	ID3D11Buffer* cbuffers[] = { g_csCountbuffer };
	g_DeviceContext->CSSetShaderResources(0, 8, srvs);
	g_DeviceContext->CSSetConstantBuffers(1, 1, cbuffers);

	g_pickingIntShader = g_ComputeSys->CreateComputeShader(_T("Shaders/PickingIntersection.fx"), nullptr, "main", nullptr);
	g_pickingCmpShader = g_ComputeSys->CreateComputeShader(_T("Shaders/PickingComp.fx"), nullptr, "main", nullptr);


	g_pickIntBuffer = g_ComputeSys->CreateConstantBuffer(sizeof(PickingData::Ray), nullptr, true);
	PickingData::CompData pickData;

	pickData.numPrimitives = std::min({ cdata.numSpheres, cdata.numTexTriangles, cdata.numTriangles });
	pickData.offset = pickData.numPrimitives % 2;
	pickData.numPrimitives = pickData.numPrimitives / 2;
	g_pickCompBuffer = g_ComputeSys->CreateConstantBuffer(sizeof(PickingData::CompData), &pickData, true);

	g_pickBuff[0] = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(PickingData::PickResult), (std::max(SceneData::maxTriangles, cdata.numTexTriangles) / 1025 + 1) * 1024, true, true, nullptr, true);
	g_pickBuff[1] = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(PickingData::PickResult), (std::max(SceneData::maxTriangles, cdata.numTexTriangles) / 1025 + 1) * 1024, true, true, nullptr, true);

	return S_OK;
}

void Shutdown()
{
	SAFE_DELETE(g_pickBuff[0]);
	SAFE_DELETE(g_pickBuff[1]);
	SAFE_RELEASE(g_pickIntBuffer);
	SAFE_RELEASE(g_pickCompBuffer);
	SAFE_RELEASE(g_sampler);
	SAFE_DELETE(g_pickingIntShader);
	SAFE_DELETE(g_pickingCmpShader);
	SAFE_DELETE(g_csSpotLightBuffer);
	SAFE_DELETE(g_csTexture1);
	SAFE_DELETE(g_csNormal1);
	SAFE_DELETE(g_csTexTriangleBuffer);
	SAFE_DELETE(g_csPointLightBuffer);
	SAFE_DELETE(g_csTriangleBuffer);
	SAFE_DELETE(g_csSphereBuffer);
	SAFE_RELEASE(g_csCameraBuffer);
	SAFE_RELEASE(g_csCountbuffer);
	SAFE_DELETE(g_Scene);
	SAFE_DELETE(g_Timer);
	SAFE_DELETE(g_ComputeShader);
	SAFE_DELETE(g_ComputeSys);
	SAFE_RELEASE(g_BackBufferUAV);
	SAFE_RELEASE(g_SwapChain);
	SAFE_RELEASE(g_DeviceContext);
	SAFE_RELEASE(g_Device);
	SAFE_DELETE(g_Input);
		
		
}

void Picking()
{
	int x, y;
	g_Input->GetMousePos(x, y);
	auto cam = g_Scene->GetCamera();
	float u = (2 * ((x + 0.5f) / g_Width) - 1) * std::tanf(cam->GetFov() / 2 * DirectX::XM_PI / 180) * cam->GetAspect();
	float v = (1 - 2 * ((y + 0.5f) / g_Height)) * tan(cam->GetFov() / 2 * DirectX::XM_PI / 180);

	XMVECTOR p1 = XMVectorSet(u*cam->GetNearP(), v*cam->GetNearP(), cam->GetNearP(), 1.0f);
	XMVECTOR p2 = XMVectorSet(u*cam->GetFarP(), v*cam->GetFarP(), cam->GetFarP(), 1.0f);
	XMMATRIX mat = XMLoadFloat4x4(&cam->GetViewInv());

	p1 = XMVector4Transform(p1, mat);
	p2 = XMVector4Transform(p2, mat);

	XMVECTOR pos = XMLoadFloat3(&cam->GetPosition());


	D3D11_MAPPED_SUBRESOURCE mappedData;
	




	g_DeviceContext->Map(g_pickIntBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	PickingData::Ray* ray = (PickingData::Ray*)mappedData.pData;

	XMStoreFloat4(&ray->dir, XMVector3Normalize(p2 - p1));
	p1 = XMVector3Normalize(p1 - pos);
	XMStoreFloat4(&ray->pos, pos);

	g_DeviceContext->Unmap(g_pickIntBuffer, 0);

	ID3D11Buffer* cbuffers[] = { g_pickIntBuffer };
	g_DeviceContext->CSSetConstantBuffers(2, 1, cbuffers);

	ID3D11UnorderedAccessView* nulluav[] = { nullptr };
	
	ID3D11UnorderedAccessView* uav[] = { g_pickBuff[0]->GetUnorderedAccessView() };

	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, uav, nullptr);
	g_pickingIntShader->Set();
	const SceneData::CountData& cdata = g_Scene->GetCounts();
	
	uint32_t maxPrim = std::max({ cdata.numSpheres, cdata.numTexTriangles, cdata.numTriangles });
	uint32_t offset = maxPrim % 2;
	uint32_t numGroups = maxPrim / 1025 + 1;;
	g_DeviceContext->Dispatch(numGroups, 1, 1);

	PickingData::PickResult data[1024];

	void* data_d = g_pickBuff[0]->MapRead<void>();
	memcpy(data, data_d, 1024 * sizeof(PickingData::PickResult));
	g_pickBuff[0]->Unmap();

	ID3D11ShaderResourceView* srvs[1];
	uint32_t halfmax = maxPrim / 2;
	g_pickingCmpShader->Set();
	uint32_t index = 0;
	while(halfmax >= 32)
	{
		srvs[0] = g_pickBuff[index % 2]->GetResourceView();
		uav[0] = g_pickBuff[(index + 1) % 2]->GetUnorderedAccessView();
		g_DeviceContext->CSSetShaderResources(5, 1, srvs);
		g_DeviceContext->CSSetUnorderedAccessViews(0, 1, uav, nullptr);

		g_DeviceContext->Map(g_pickCompBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		PickingData::CompData* cdata_d = (PickingData::CompData*)mappedData.pData;

		cdata_d->numPrimitives = halfmax;
		cdata_d->offset = offset;

		g_DeviceContext->Unmap(g_pickCompBuffer, 0);

		ID3D11Buffer* cbuffers[] = { g_pickCompBuffer };
		g_DeviceContext->CSSetConstantBuffers(2, 1, cbuffers);
		

		numGroups = halfmax / 1025 + 1;

		g_DeviceContext->Dispatch(numGroups, 1, 1);


		data_d = g_pickBuff[(index + 1) % 2]->MapRead<void>();
		memcpy(data, data_d, 1024 * sizeof(PickingData::PickResult));
		g_pickBuff[(index + 1) % 2]->Unmap();

		halfmax /= 2;
		index++;

		
	}
	g_pickingCmpShader->Unset();
	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, nulluav, nullptr);

	float t = data[0].t;
	int ID = data[0].ID;
	uint32_t Type = data[0].type;
	XMFLOAT3 fpos = data[0].Pos;
	for (uint32_t i = 1; i < offset + halfmax*2; i++)
	{
		if (data[i].t < t)
		{
			t = data[i].t;
			ID = data[i].ID;
			Type = data[i].type;
			fpos = data[i].Pos;
		}
	}
	

	// Do something with the picking. 
	if (ID >= 0)
	{
		switch (Type)
		{
		case 0:
		{
			g_Scene->UpdateSphere(ID);
			break;
		}
		case 1:
		{
			g_Scene->UpdateTriangle(ID);
			break;
		}
		case 2:
		{
			g_Scene->UpdateTexTriangle(fpos);
			break;
		}
		}
		const SceneData::CountData& cdata2 = g_Scene->GetCounts();
		g_DeviceContext->Map(g_csCountbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
		memcpy(mappedData.pData, &cdata2, sizeof(cdata2));
		g_DeviceContext->Unmap(g_csCountbuffer, 0);
		// Copy sphere data to device
		const SceneData::Sphere& sphereData_h = g_Scene->GetSpheres();
		void* sphereData_d = g_csSphereBuffer->Map<void>();

		memcpy(sphereData_d, sphereData_h.Position3_Radius_1, sizeof(XMFLOAT4)*cdata2.numSpheres);
		sphereData_d = (XMFLOAT4*)sphereData_d + SceneData::maxSpheres;

		memcpy(sphereData_d, sphereData_h.Color, sizeof(XMFLOAT4)*cdata2.numSpheres);

		g_csSphereBuffer->Unmap();


		// Copy triangle data to device
		const SceneData::Triangle& triangleData_h = g_Scene->GetTriangles();

		void* triangleData_d = g_csTriangleBuffer->Map<void>();

		memcpy(triangleData_d, triangleData_h.p0, sizeof(XMFLOAT4)*cdata2.numTriangles);
		triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

		memcpy(triangleData_d, triangleData_h.p1, sizeof(XMFLOAT4)*cdata2.numTriangles);
		triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

		memcpy(triangleData_d, triangleData_h.p2, sizeof(XMFLOAT4)*cdata2.numTriangles);
		triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

		memcpy(triangleData_d, triangleData_h.Color, sizeof(XMFLOAT4)*cdata2.numTriangles);

		g_csTriangleBuffer->Unmap();



		ID3D11ShaderResourceView* srvs[] = {
			g_csSphereBuffer->GetResourceView(),
			g_csTriangleBuffer->GetResourceView(),
			g_csPointLightBuffer->GetResourceView(),
			g_csTexTriangleBuffer->GetResourceView(),
			g_csSpotLightBuffer->GetResourceView(),
			nullptr,
			g_csTexture1->GetResourceView(),
			g_csNormal1->GetResourceView() 
		};
		g_DeviceContext->CSSetShaderResources(0, 8, srvs);
	}
	

	
}



HRESULT Update(float deltaTime)
{
	if (g_Input->IsKeyPushed(Input::Keys::RButton))
	{
		Picking();
	}

	uint8_t change = g_Scene->Update(deltaTime);
	
	if (change)
	{
		const SceneData::CountData& cdata = g_Scene->GetCounts();

		// Copy sphere data to device
		const SceneData::Sphere& sphereData_h = g_Scene->GetSpheres();
		void* sphereData_d = g_csSphereBuffer->Map<void>();

		memcpy(sphereData_d, sphereData_h.Position3_Radius_1, sizeof(XMFLOAT4)*cdata.numSpheres);
		sphereData_d = (XMFLOAT4*)sphereData_d + SceneData::maxSpheres;

		memcpy(sphereData_d, sphereData_h.Color, sizeof(XMFLOAT4)*cdata.numSpheres);

		g_csSphereBuffer->Unmap();


		// Copy triangle data to device
		const SceneData::Triangle& triangleData_h = g_Scene->GetTriangles();

		void* triangleData_d = g_csTriangleBuffer->Map<void>();

		memcpy(triangleData_d, triangleData_h.p0, sizeof(XMFLOAT4)*cdata.numTriangles);
		triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

		memcpy(triangleData_d, triangleData_h.p1, sizeof(XMFLOAT4)*cdata.numTriangles);
		triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

		memcpy(triangleData_d, triangleData_h.p2, sizeof(XMFLOAT4)*cdata.numTriangles);
		triangleData_d = (XMFLOAT4*)triangleData_d + SceneData::maxTriangles;

		memcpy(triangleData_d, triangleData_h.Color, sizeof(XMFLOAT4)*cdata.numTriangles);

		g_csTriangleBuffer->Unmap();

		// Copy pointlight data to device
		const SceneData::PointLight& pointLightData_h = g_Scene->GetPointLights();
		void* pointLightData_d = g_csPointLightBuffer->Map<void>();

		memcpy(pointLightData_d, pointLightData_h.Position3_Luminosity1, sizeof(XMFLOAT4)*cdata.numPointLights);

		g_csPointLightBuffer->Unmap();




		ID3D11ShaderResourceView* srvs[] = { 
			g_csSphereBuffer->GetResourceView(),
			g_csTriangleBuffer->GetResourceView(),
			g_csPointLightBuffer->GetResourceView(),
			g_csTexTriangleBuffer->GetResourceView(),
			g_csSpotLightBuffer->GetResourceView(),
			nullptr,
			g_csTexture1->GetResourceView(),
			g_csNormal1->GetResourceView()
		};
		g_DeviceContext->CSSetShaderResources(0, 8, srvs);
	}

	
	g_Input->Frame();

	return S_OK;
}

HRESULT Render(float deltaTime)
{

	D3D11_MAPPED_SUBRESOURCE mappedData;
	g_DeviceContext->Map(g_csCameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	SceneData::CameraData* data = (SceneData::CameraData*)mappedData.pData;
	data->Pos = g_Scene->GetCamera()->GetPosition();
	data->Forward = g_Scene->GetCamera()->GetForward();
	XMMATRIX inv = XMLoadFloat4x4(&g_Scene->GetCamera()->GetViewInv());
	inv = XMMatrixTranspose(inv); // Don't know if i actually have to transpose.
	XMStoreFloat4x4(&data->ViewInv, inv); 

	data->Aspect = g_Scene->GetCamera()->GetAspect();
	data->Fov = g_Scene->GetCamera()->GetFov();
	data->ScreenWidth = g_Width;
	data->ScreenHeight = g_Height;
	data->NearP = g_Scene->GetCamera()->GetNearP();
	data->FarP = g_Scene->GetCamera()->GetFarP();

	g_DeviceContext->Unmap(g_csCameraBuffer, 0);

	ID3D11UnorderedAccessView* nulluav[] = { nullptr };
	ID3D11UnorderedAccessView* uav[] = { g_BackBufferUAV };

	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, uav, nullptr);
	g_DeviceContext->CSSetConstantBuffers(0, 1, &g_csCameraBuffer);
	g_ComputeShader->Set();
	

	g_Timer->Start();
	g_DeviceContext->Dispatch( 50, 25, 1 );
	g_Timer->Stop();




	g_ComputeShader->Unset();

	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, nulluav, nullptr);

	if(FAILED(g_SwapChain->Present( 0, 0 )))
		return E_FAIL;



	return S_OK;
}

//--------------------------------------------------------------------------------------
// Entry point to the program. Initializes everything and goes into a message processing 
// loop. Idle time is used to render the scene.
//--------------------------------------------------------------------------------------
int WINAPI wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
{
#if defined( DEBUG ) || defined( _DEBUG )
	_CrtDumpMemoryLeaks();
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
	//_crtBreakAlloc = 242;
#endif

	if( FAILED( InitWindow( hInstance, nCmdShow ) ) )
		return 0;

	if( FAILED( Init() ) )
		return 0;

	__int64 cntsPerSec = 0;
	QueryPerformanceFrequency((LARGE_INTEGER*)&cntsPerSec);
	float secsPerCnt = 1.0f / (float)cntsPerSec;

	__int64 prevTimeStamp = 0;
	QueryPerformanceCounter((LARGE_INTEGER*)&prevTimeStamp);


 //g_Input->LockMouseToCenter(true);

 uint32_t frameCount = 0;
 uint32_t pCount = 0;
 float timeval = 0.0f;
	// Main message loop
	MSG msg = {0};
	while(WM_QUIT != msg.message)
	{
		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			__int64 currTimeStamp = 0;
			QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
			float dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;

			

			//render
			Update(dt);
			Render(dt);

			

			prevTimeStamp = currTimeStamp;

			timeval += dt;
			frameCount++;
			if (timeval > 1.0f)
			{
				pCount = frameCount;
				frameCount = 0;
				timeval = 0.0f;
			}

			char title[256];
			sprintf_s(
				title,
				sizeof(title),
				"FPS: %d, Dispatch time: %f", pCount,
				g_Timer->GetTime()
			);
			SetWindowTextA(g_hWnd, title);
		}
	}


	//for (int i = 0; i < 1000; i++)
	//{
	//	__int64 currTimeStamp = 0;
	//	QueryPerformanceCounter((LARGE_INTEGER*)&currTimeStamp);
	//	float dt = (currTimeStamp - prevTimeStamp) * secsPerCnt;



	//	//render
	//	g_Timer->Start();
	//	Update(dt);
	//	g_Timer->Stop();
	//	updatetime += g_Timer->GetTime();
	//	

	//	g_Timer->Start();
	//	Render(dt);
	//	g_Timer->Stop();
	//	rendertime += g_Timer->GetTime();

	//	prevTimeStamp = currTimeStamp;
	//}

	//updatetime /= 1000;
	//rendertime /= 1000;
	//Shutdown();


	//if (AllocConsole())
	//{
	//	freopen("conin$", "r", stdin);
	//	freopen("conout$", "w", stdout);
	//	freopen("conout$", "w", stderr);

	//	printf("<----||Console Initialized||---->\n\n");


	//	printf("Update time: %f\n", updatetime);
	//	printf("Render time: %f\n", rendertime);

	//	getchar();
	//}

	return (int) msg.wParam;
}


//--------------------------------------------------------------------------------------
// Register class and create window
//--------------------------------------------------------------------------------------
HRESULT InitWindow( HINSTANCE hInstance, int nCmdShow )
{


	// Register class
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style          = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc    = WndProc;
	wcex.cbClsExtra     = 0;
	wcex.cbWndExtra     = 0;
	wcex.hInstance      = hInstance;
	wcex.hIcon          = 0;
	wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName   = NULL;
	wcex.lpszClassName  = _T("BTH_D3D_Template");
	wcex.hIconSm        = 0;
	if( !RegisterClassEx(&wcex) )
		return E_FAIL;

	// Create window
	g_hInst = hInstance; 

	g_Width = 800;
	g_Height = 800;

	RECT rc = { 0, 0, g_Width, g_Height };
	AdjustWindowRect( &rc, WS_OVERLAPPEDWINDOW, FALSE );
	
	if(!(g_hWnd = CreateWindow(
							_T("BTH_D3D_Template"),
							_T("BTH - Direct3D 11.0 Template"),
							WS_OVERLAPPEDWINDOW,
							CW_USEDEFAULT,
							CW_USEDEFAULT,
							rc.right - rc.left,
							rc.bottom - rc.top,
							NULL,
							NULL,
							hInstance,
							NULL)))
	{
		return E_FAIL;
	}

	g_Input = new Input;
	g_Input->Init(g_hWnd, g_Width, g_Height);

	ShowWindow( g_hWnd, nCmdShow );
	SetForegroundWindow(g_hWnd);
	SetFocus(g_hWnd);
	
	return S_OK;
}


//--------------------------------------------------------------------------------------
// Called every time the application receives a message
//--------------------------------------------------------------------------------------
LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message) 
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	/*case WM_KEYDOWN:

		switch(wParam)
		{
			case VK_ESCAPE:
				PostQuitMessage(0);
				break;
		}
		break;*/

	default:
		return g_Input->MessageHandler(hWnd, message, wParam, lParam);
	}

	return 0;
}