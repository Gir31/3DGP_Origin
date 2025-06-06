#pragma once


#include<string>
#include<wrl.h>
#include<shellapi.h>

#include<d3d12.h> //Direct3D 12API �Լ��� ����ϱ� ���� �ݵ�� ���Խ��Ѿ� �ϴ� ��� ����
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

#define FRAME_BUFFER_WIDTH 800
#define FRAME_BUFFER_HEIGHT 600