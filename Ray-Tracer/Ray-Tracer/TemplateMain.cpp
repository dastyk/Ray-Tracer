//--------------------------------------------------------------------------------------
// File: TemplateMain.cpp
//
// BTH-D3D-Template
//
// Copyright (c) Stefan Petersson 2013. All rights reserved.
//--------------------------------------------------------------------------------------
#include "stdafx.h"

#include "ComputeHelp.h"
#include "D3D11Timer.h"
#include "Input.h"
#include "Scene.h"

using namespace DirectX;

#if defined( DEBUG ) || defined( _DEBUG )
#define _CRTDBG_MAP_ALLOC  
#include <stdlib.h>  
#include <crtdbg.h>  
#endif

#include <stdint.h>

/*	DirectXTex library - for usage info, see http://directxtex.codeplex.com/
	
	Usage example (may not be the "correct" way, I just wrote it in a hurry):

	DirectX::ScratchImage img;
	DirectX::TexMetadata meta;
	ID3D11ShaderResourceView* srv = nullptr;
	if(SUCCEEDED(hr = DirectX::LoadFromDDSFile(_T("C:\\Program Files (x86)\\Microsoft DirectX SDK (June 2010)\\Samples\\Media\\Dwarf\\Armor.dds"), 0, &meta, img)))
	{
		//img loaded OK
		if(SUCCEEDED(hr = DirectX::CreateShaderResourceView(g_Device, img.GetImages(), img.GetImageCount(), meta, &srv)))
		{
			//srv created OK
		}
	}
*/
#include <DirectXTex.h>

#if defined( DEBUG ) || defined( _DEBUG )
#pragma comment(lib, "DirectXTexD.lib")
#else
#pragma comment(lib, "DirectXTex.lib")
#endif

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
ComputeBuffer*				g_csPointLightBuffer	= nullptr;


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
	GetClientRect( g_hWnd, &rc );
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
	ZeroMemory( &sd, sizeof(sd) );
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

	for( UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++ )
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

		if( SUCCEEDED( hr ) )
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
	if( FAILED(hr) )
		return hr;

	// Create a render target view
	ID3D11Texture2D* pBackBuffer;
	hr = g_SwapChain->GetBuffer( 0, __uuidof( ID3D11Texture2D ), (LPVOID*)&pBackBuffer );
	if( FAILED(hr) )
		return hr;
	
	// create shader unordered access view on back buffer for compute shader to write into texture
	hr = g_Device->CreateUnorderedAccessView( pBackBuffer, nullptr, &g_BackBufferUAV );

	// Release, not needed anymore.
	pBackBuffer->Release();

	//create helper sys and compute shader instance
	g_ComputeSys = new ComputeWrap(g_Device, g_DeviceContext);
	g_ComputeShader = g_ComputeSys->CreateComputeShader(_T("Shaders/RayTracer.fx"), nullptr, "main", nullptr);
	g_Timer = new D3D11Timer(g_Device, g_DeviceContext);


	g_Scene = new Scene(g_Width, g_Height, *g_Input);


	const CountData * cdata = g_Scene->GetCounts();
	g_csCountbuffer = g_ComputeSys->CreateConstantBuffer(sizeof(CountData), (void*)cdata, true);

	g_csCameraBuffer = g_ComputeSys->CreateConstantBuffer(sizeof(CameraData), nullptr, true);


	g_csSphereBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(Sphere), Scene::maxSpheres, true, false, nullptr, true);
	
	g_csPointLightBuffer = g_ComputeSys->CreateBuffer(COMPUTE_BUFFER_TYPE::STRUCTURED_BUFFER, sizeof(PointLight), Scene::maxPointLights, true, false, nullptr, true);

	void* sm = g_csSphereBuffer->Map<void>();
	memcpy(sm, g_Scene->GetSpheres(), cdata->numSpheres * sizeof(Sphere));
	g_csSphereBuffer->Unmap();

	sm = g_csPointLightBuffer->Map<void>();
	memcpy(sm, g_Scene->GetPointLights(), cdata->numPointLights * sizeof(PointLight));
	g_csPointLightBuffer->Unmap();

	ID3D11ShaderResourceView* srv[] = { g_csSphereBuffer->GetResourceView() , g_csPointLightBuffer->GetResourceView() };

	g_DeviceContext->CSSetShaderResources(0, 2, srv);
	g_DeviceContext->CSSetConstantBuffers(0, 1, &g_csCountbuffer);
	return S_OK;
}

void Shutdown()
{	
	SAFE_DELETE(g_csPointLightBuffer);
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

HRESULT Update(float deltaTime)
{
	g_Input->Frame();

	g_Scene->Update(deltaTime);
	
	return S_OK;
}

HRESULT Render(float deltaTime)
{

	D3D11_MAPPED_SUBRESOURCE mappedData;
	g_DeviceContext->Map(g_csCameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
	CameraData* data = (CameraData*)mappedData.pData;
	data->Pos = g_Scene->GetCamera()->GetPosition();
	data->Forward = g_Scene->GetCamera()->GetForward();
	XMMATRIX inv = XMLoadFloat4x4(&g_Scene->GetCamera()->GetViewInv());
	inv = XMMatrixTranspose(inv);
	XMStoreFloat4x4(&data->ViewInv, inv); // Maybe need to transpose.

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
	g_DeviceContext->CSSetConstantBuffers(1, 1, &g_csCameraBuffer);
	g_ComputeShader->Set();
	

	g_Timer->Start();
	g_DeviceContext->Dispatch( 25, 25, 1 );
	g_Timer->Stop();
	g_ComputeShader->Unset();

	g_DeviceContext->CSSetUnorderedAccessViews(0, 1, nulluav, nullptr);

	if(FAILED(g_SwapChain->Present( 0, 0 )))
		return E_FAIL;


	char title[256];
	sprintf_s(
		title,
		sizeof(title),
		"BTH - DirectCompute DEMO - Dispatch time: %f",
		g_Timer->GetTime()
	);
	SetWindowTextA(g_hWnd, title);

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


 g_Input->LockMouseToCenter(true);

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
		}
	}

	Shutdown();

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