#pragma once


#include<string>
#include<wrl.h>
#include<shellapi.h>
#include<tchar.h>

#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib")

#include<d3d12.h> //Direct3D 12API 함수를 사용하기 위해 반드시 포함시켜야 하는 헤더 파일
#include<dxgi1_4.h>
#include<D3Dcompiler.h>

#include<DirectXMath.h>
#include<DirectXPackedVector.h>
#include<DirectXColors.h>

#include<DirectXCollision.h>
#include<DXGIDebug.h>

using namespace DirectX;
using namespace DirectX::PackedVector;

using Microsoft::WRL::ComPtr;


#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#pragma comment(lib, "dxguid.lib")
//#include <windows.h>   // 윈도우 기본 정의
//#include <dxgi1_6.h>   // DXGI 포맷, IDXGIFactory 등

#define _WITH_SWAPCHAIN_FULLSCREEN_STATE
#define FRAME_BUFFER_WIDTH 800
#define FRAME_BUFFER_HEIGHT 600