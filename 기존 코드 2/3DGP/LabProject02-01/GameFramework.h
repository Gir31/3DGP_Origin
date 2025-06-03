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
	CGameFramework();
	~CGameFramework();

private:
	// 윈도우 핸들 및 크기 정보
	HINSTANCE m_hInstance;                              // 애플리케이션 인스턴스 핸들
	HWND      m_hWnd;                                   // 윈도우 핸들
	int       m_nWndClientWidth;                        // 클라이언트 영역 너비
	int       m_nWndClientHeight;                       // 클라이언트 영역 높이

	// Direct3D 12 장치 및 DXGI 팩토리
	IDXGIFactory4* m_pdxgiFactory;						// DXGI 팩토리 (디바이스 생성에 필요)
	IDXGISwapChain3* m_pdxgiSwapChain;					// 스왑체인 (버퍼 전환용)
	ID3D12Device* m_pd3dDevice;							// D3D12 디바이스 (GPU 자원 생성 담당)

	// MSAA 설정
	bool m_bMsaa4xEnable = false;                       // 4x MSAA 사용 여부
	UINT m_nMsaa4xQualityLevels = 0;                    // MSAA 품질 레벨 수 (지원 수준)

	// 스왑체인 및 렌더타겟 설정
	static const UINT m_nSwapChainBuffers = 2;          // 스왑체인 버퍼 수 (더블 버퍼링)
	UINT m_nSwapChainBufferIndex;                       // 현재 사용 중인 백버퍼 인덱스

	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers]; // 렌더타겟용 백버퍼 리소스
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;      // RTV(렌더타겟뷰) 디스크립터 힙
	UINT m_nRtvDescriptorIncrementSize;                 // RTV 디스크립터 간격 크기

	// 깊이 스텐실 버퍼
	ID3D12Resource* m_pd3dDepthStencilBuffer;			// 깊이/스텐실용 버퍼 리소스
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;      // DSV(깊이/스텐실 뷰) 디스크립터 힙
	UINT m_nDsvDescriptorIncrementSize;                 // DSV 디스크립터 간격 크기

	// 커맨드 큐, 리스트, 할당자
	ID3D12CommandAllocator* m_pd3dCommandAllocator;		// 명령 리스트 메모리 관리자
	ID3D12CommandQueue* m_pd3dCommandQueue;				// GPU 명령 큐
	ID3D12GraphicsCommandList* m_pd3dCommandList;		// 명령 리스트 (그리기 명령 담음)

	// 동기화 객체 (GPU-CPU 동기 맞추기)
	ID3D12Fence* m_pd3dFence;							// GPU 작업 완료 여부 확인용 펜스
	UINT64        m_nFenceValues[m_nSwapChainBuffers];  // 버퍼별 펜스 값
	HANDLE        m_hFenceEvent;                        // 펜스 이벤트 핸들

#if defined(_DEBUG)
	ID3D12Debug* m_pd3dDebugController;					// D3D 디버깅 레이어
#endif

	// 게임 객체
	std::array<CPlayer*, 4> m_pPlayer;                  // 최대 4명의 플레이어 포인터 배열
	CGameObject* m_pAutoAttackObject = NULL;            // 자동 공격 대상
	CGameObject* m_pLockedObject = NULL;                // 현재 락온한 적 객체

	StageManager* manager = NULL;                       // 현재 스테이지/씬을 관리하는 매니저 클래스

	CGameTimer m_GameTimer;                             // 전체 프레임 타이머

	POINT m_ptOldCursorPos;                             // 이전 프레임 기준 마우스 커서 위치
	_TCHAR m_pszFrameRate[50];                          // 프레임 속도 문자열 버퍼

	bool m_bShieldActive = false;                       // 플레이어 쉴드 활성 상태 여부


public:
	// 시스템 초기화 및 종료
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);   // 전체 시스템 생성
	void OnDestroy();                                    // 전체 시스템 종료 및 리소스 해제

	// Direct3D 12 초기화 관련
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void ChangeSwapChainState();                         // 풀스크린 토글 등

	// 게임 월드 구성
	void BuildObjects();                                 // 씬/오브젝트 생성
	void ReleaseObjects();                               // 씬/오브젝트 해제

	// 게임 루프 및 업데이트
	void ProcessInput();                                 // 입력 처리
	void AnimateObjects();                               // 오브젝트 애니메이션 (변환 등)
	void FrameAdvance();                                 // 한 프레임 처리 (업데이트 + 렌더링)

	// 셰이더 변수 처리
	virtual void CreateShaderVariables();                // CBV 등 생성
	virtual void ReleaseShaderVariables();               // CBV 해제
	virtual void UpdateShaderVariables();                // CBV 등 값 업데이트

	// GPU 동기화 관련
	void WaitForGpuComplete();                           // GPU 명령 완료 대기
	void MoveToNextFrame();                              // 프레임 전환 (다음 백버퍼로)

	// 입력 메시지 처리
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

