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
private:
	//////////////////////////////////////////
	// 윈도우 관련
	HINSTANCE						m_hInstance = nullptr;
	HWND							m_hWnd = nullptr;
	int								m_nWndClientWidth;
	int								m_nWndClientHeight;

	//////////////////////////////////////////
	// Direct3D 12 기본 구성
	IDXGIFactory4*					m_pdxgiFactory;							// DXGI 팩토리
	IDXGISwapChain3*				m_pdxgiSwapChain;						// 스왑 체인
	ID3D12Device*					m_pd3dDevice;							// 디바이스

	//////////////////////////////////////////
	// MSAA 설정
	bool							m_bMsaa4xEnable = false;				// 4x MSAA 사용 여부
	UINT							m_nMsaa4xQualityLevels = 0;				// MSAA 품질 수준

	//////////////////////////////////////////
	// 스왑 체인 버퍼
	static const UINT				m_nSwapChainBuffers = 2;				// 스왑 체인 후면 버퍼 개수
	UINT							m_nSwapChainBufferIndex;				// 현재 사용 중인 버퍼 인덱스
	ID3D12Resource*					m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];	// 렌더 타겟 버퍼
	ID3D12DescriptorHeap*			m_pd3dRtvDescriptorHeap;				// RTV 서술자 힙
	UINT							m_nRtvDescriptorIncrementSize;			// RTV 서술자 증가 크기

	//////////////////////////////////////////
	// 깊이/스텐실 버퍼
	ID3D12Resource*					m_pd3dDepthStencilBuffer;				// 깊이 스텐실 버퍼
	ID3D12DescriptorHeap*			m_pd3dDsvDescriptorHeap;				// DSV 서술자 힙
	UINT							m_nDsvDescriptorIncrementSize;			// DSV 서술자 증가 크기

	//////////////////////////////////////////
	// 커맨드 객체
	ID3D12CommandQueue*				m_pd3dCommandQueue;
	ID3D12CommandAllocator*			m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList*		m_pd3dCommandList;

	//////////////////////////////////////////
	// 파이프라인 상태 및 동기화
	ID3D12PipelineState*			m_pd3dPipelineState;					// 파이프라인 상태 객체
	ID3D12Fence*					m_pd3dFence;							// 펜스 객체
	UINT64							m_nFenceValue[m_nSwapChainBuffers];		// 펜스 값
	HANDLE							m_hFenceEvent;							// 펜스 이벤트 핸들

	//////////////////////////////////////////
	// 뷰포트 및 스크린
	D3D12_VIEWPORT					m_d3dViewport = {};
	D3D12_RECT						m_d3dScissorRect = {};

	//////////////////////////////////////////
	// 게임 플레이 구성
	std::array<CPlayer*, 4>			m_pPlayer = {};
	CGameObject*					m_pAutoAttackObject = nullptr;
	CGameObject*					m_pLockedObject = nullptr;
	StageManager*					manager = nullptr;

	//////////////////////////////////////////
	// 시스템 유틸리티
	CGameTimer						m_GameTimer;
	POINT							m_ptOldCursorPos = {};
	_TCHAR							m_pszFrameRate[50] = { 0 };

	//////////////////////////////////////////
	// 게임 상태
	bool							m_bActive = true;
	bool							m_bShieldActive = false;
public:
	// 생성자 / 소멸자
	CGameFramework();
	~CGameFramework();

	// 초기화 / 해제
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	///////////////////////////////////////////////
	// Direct3D 12 초기화 관련
	void CreateDirect3DDevice();                  // 디바이스 생성
	void CreateCommandQueueAndList();             // 명령 큐, 할당자, 리스트 생성
	void CreateSwapChain();                       // 스왑 체인 생성
	void CreateRtvAndDsvDescriptorHeaps();        // RTV/DSV 서술자 힙 생성
	void CreateRenderTargetViews();               // 렌더 타겟 뷰 생성
	void CreateDepthStencilView();                // 깊이-스텐실 뷰 생성

	///////////////////////////////////////////////
	// 프레임 버퍼 및 렌더링
	void BuildFrameBuffer();                      // 프레임 버퍼 구성
	void ClearFrameBuffer(DWORD dwColor);         // 화면 초기화
	void PresentFrameBuffer();                    // 화면 출력 (Present)

	///////////////////////////////////////////////
	// 게임 오브젝트 관리
	void BuildObjects();                          // 게임 오브젝트 생성
	void ReleaseObjects();                        // 게임 오브젝트 해제

	///////////////////////////////////////////////
	// 게임 루프 처리
	void ProcessInput();                          // 입력 처리
	void AnimateObjects();                        // 애니메이션 및 상태 갱신
	void FrameAdvance();                          // 프레임 전환 처리

	///////////////////////////////////////////////
	// GPU 동기화
	void WaitForGpuComplete();                    // GPU 작업 완료 대기
	void MoveToNextFrame();

	///////////////////////////////////////////////
	// 입력 메시지 처리
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	///////////////////////////////////////////////
	// 기타
	void SetActive(bool bActive) { m_bActive = bActive; }
};

