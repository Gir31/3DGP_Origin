#pragma once

#include "Player.h"
#include "Scene.h"
#include "Title.h"
#include "Menu.h"
#include "Stage1.h"
#include "Stage2.h"
#include "StageManager.h"
#include "Timer.h"

class CGameFramework
{
public:
	CGameFramework() { }
	~CGameFramework() { }

private:
	HINSTANCE					m_hInstance = NULL;
	HWND						m_hWnd = NULL;

	bool						m_bActive = true;

	RECT						m_rcClient;

	std::array<CPlayer*, 4>		m_pPlayer;
	CGameObject*				m_pAutoAttackObject = NULL;
	CGameObject*				m_pLockedObject = NULL;

	StageManager*				manager = NULL;

	CGameTimer					m_GameTimer;

	POINT						m_ptOldCursorPos;

	_TCHAR						m_pszFrameRate[50];

	bool						m_bShieldActive = false;

	// [추가]
	 // D3D12 디바이스 및 스왑체인
	IDXGIFactory4* m_pdxgiFactory = nullptr;
	ID3D12Device* m_pd3dDevice = nullptr;
	IDXGISwapChain3* m_pdxgiSwapChain = nullptr;

	// 백버퍼 설정
	static const UINT           m_nSwapChainBuffers = 2;
	UINT                        m_nSwapChainBufferIndex = 0;
	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers] = {};

	// RTV(렌더 타겟 뷰) 설정
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap = nullptr;
	UINT                        m_nRtvDescriptorIncrementSize = 0;

	// DSV(깊이 스텐실 뷰)
	ID3D12Resource* m_pd3dDepthStencilBuffer = nullptr;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap = nullptr;
	UINT                        m_nDsvDescriptorIncrementSize = 0;

	// 커맨드 관련
	ID3D12CommandQueue* m_pd3dCommandQueue = nullptr;
	ID3D12CommandAllocator* m_pd3dCommandAllocator = nullptr;
	ID3D12GraphicsCommandList* m_pd3dCommandList = nullptr;

	// GPU 동기화용 펜스
	ID3D12Fence* m_pd3dFence = nullptr;
	UINT64                      m_nFenceValues[m_nSwapChainBuffers] = {};
	HANDLE                      m_hFenceEvent = nullptr;

#if defined(_DEBUG)
	ID3D12Debug* m_pd3dDebugController = nullptr;
#endif

public:
	void OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void SetActive(bool bActive) { m_bActive = bActive; }

	// [추가]
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
};

