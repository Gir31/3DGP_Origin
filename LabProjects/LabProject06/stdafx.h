#pragma once


#include<string>
#include<wrl.h>
#include<shellapi.h>
#include<tchar.h>

#include <mmsystem.h> 
#pragma comment(lib, "winmm.lib")

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
//#include <windows.h>   // ������ �⺻ ����
//#include <dxgi1_6.h>   // DXGI ����, IDXGIFactory ��

#define _WITH_SWAPCHAIN_FULLSCREEN_STATE
#define FRAME_BUFFER_WIDTH 800
#define FRAME_BUFFER_HEIGHT 600


extern ID3D12Resource* CreateBufferResource(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pData, UINT nBytes,
											D3D12_HEAP_TYPE d3dHeapType = D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATES d3dResourceStates = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
											ID3D12Resource** ppd3dUploadBuffer = NULL);