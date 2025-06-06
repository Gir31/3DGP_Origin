#pragma once
#include "stdafx.h"
#include "Timer.h"
#include "Scene.h"
#include "Camera.h"
#include "Player.h"
#include "StageManager.h"

class CGameFramework
{
private:
	//다음은 게임 프레임워크에서 사용할 타이머이다.
	CGameTimer m_GameTimer;
	//다음은 프레임 레이트를 주 윈도우의 캡션에 출력하기 위한 문자열이다.
	_TCHAR m_pszFrameRate[50];

	HINSTANCE m_hInstance;
	HWND m_hWnd;
	int m_nWndClientWidth;
	int m_nWndClientHeight;

	//Direct3D와 운영체제(윈도우)의 디스플레이 시스템을 연결해주는 중간 계층 API
	IDXGIFactory4* m_pdxgiFactory;
	//DXGI 팩토리 인터페이스에 대한 포인터이다. 

	//렌더링 결과를 보여주는 버퍼 교체 시스템
	IDXGISwapChain3* m_pdxgiSwapChain;
	//스왑 체인 인터페이스에 대한 포인터이다. 주로 디스플레이를 제어하기 위하여 필요하다.

	//GPU가 일을 할 수 있도록 명령을 준비시키는 핵심 인터페이스(GPU가 실행할 명령어를 큐에 넣어둠)
	ID3D12Device* m_pd3dDevice;
	//Direct3D 디바이스 인터페이스에 대한 포인터이다. 주로 리소스를 생성하기 위하여 필요하다.

	//MSAA : 렌더링 결과의 **계단 현상(jaggies)**을 줄이기 위한 기술
	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0;
	//MSAA 다중 샘플링을 활성화하고 다중 샘플링 레벨을 설정한다.

	static const UINT m_nSwapChainBuffers = 2;
	//스왑 체인의 후면 버퍼의 개수이다.

	UINT m_nSwapChainBufferIndex;
	//현재 스왑 체인의 후면 버퍼 인덱스이다.

	//사용할 렌더 타겟들을 보관하는 배열
	ID3D12Resource* m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap; //Rtv인 Descriptor를 배열로 저장하는 배열
	UINT m_nRtvDescriptorIncrementSize;
	//렌더 타겟 버퍼, 서술자 힙 인터페이스 포인터, 렌더 타겟 서술자 원소의 크기이다.
	//서술자(Descriptor) : GPU 리소스(텍스처, 버퍼 등)를 사용하는 방법에 대한 정보야.
	//즉, GPU가 "이 리소스를 이렇게 사용해라"라고 이해할 수 있게 도와주는 뷰(View) 같은 것.


	ID3D12Resource* m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;
	//깊이-스텐실 버퍼, 서술자 힙 인터페이스 포인터, 깊이-스텐실 서술자 원소의 크기이다.

	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12CommandAllocator* m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	//명령 큐, 명령 할당자, 명령 리스트 인터페이스 포인터이다.

	//렌더링할 때 필요한 모든 파이프라인 구성 요소를 GPU에 미리 설정해두는 역할
	ID3D12PipelineState* m_pd3dPipelineState;
	//그래픽스 파이프라인 상태 객체에 대한 인터페이스 포인터이다.

	//GPU의 명령 큐(그래픽스 작업) 실행 상태를 감시하거나 CPU와 GPU 간에 동기화할 때 사용함.
	//"GPU가 여기까지 했는지 ? ” 물어볼 수 있게 해줌
	ID3D12Fence* m_pd3dFence;
	//펜스의 현재 기준 값. 항상 값을 비교해서 동기화시켜줌
	UINT64 m_nFenceValues[m_nSwapChainBuffers];

	CScene* m_pScene;

	//CPU가 GPU의 작업 완료를 기다리기 위해 사용하는 이벤트 핸들
	HANDLE m_hFenceEvent;
	//펜스 인터페이스 포인터, 펜스의 값, 이벤트 핸들이다.

	StageManager* manager = NULL;

	//뷰포트와 씨저 사각형이다. 
	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
public:
	CGameFramework();
	~CGameFramework();

	CCamera* m_pCamera = NULL;

	void MoveToNextFrame();

	void ChangeSwapChainState();

	//어떤 객체(혹은 애플리케이션)의 초기화 작업을 담당하는 함수. 즉, 게임을 실행할 때 미리 셋팅해두어야하는 리소스 및 데이터들로 초기화해주겠다는 것이다.
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	//프레임워크를 초기화하는 함수이다(주 윈도우가 생성되면 호출된다). 

	void OnDestroy();//생성한 리소스들 해제하는 곳. 게임 종료 시 실행
	void CreateSwapChain(); //스왑체인을 만드는 함수
	void CreateRtvAndDsvDescriptorHeaps(); //렌더 타겟(RTV) & 깊이 스텐실(DSV) 뷰용 디스크립터 힙 생성.
	void CreateDirect3DDevice(); //ID3D12Device를 생성해서 GPU와 통신할 수 있는 객체를 만든다.
	void CreateCommandQueueAndList(); //GPU에게 명령을 보낼 커맨드 큐와 커맨드 리스트를 만드는 곳
	//스왑 체인, 디바이스, 서술자 힙, 명령 큐/할당자/리스트를 생성하는 함수이다.     

	void CreateRenderTargetViews(); //GPU가 색을 그려 넣는 용도로 접근할 수 있도록 함
	void CreateDepthStencilView();  //깊이/스텐실 테스트 용도로 텍스처 접근
	//렌더 타겟 뷰와 깊이-스텐실 뷰를 생성하는 함수이다. 

	void BuildObjects();
	void ReleaseObjects();
	//렌더링할 메쉬와 게임 객체를 생성하고 소멸하는 함수이다. 
   //프레임워크의 핵심(사용자 입력, 애니메이션, 렌더링)을 구성하는 함수이다. 

	void ProcessInput(); //사용자 입력 (키보드, 마우스, 컨트롤러 등)을 처리하는 함수
	void AnimateObjects(); //게임 내 오브젝트(플레이어, 적, 배경 등)에 애니메이션 적용
	void FrameAdvance(); //한 프레임의 전체 흐름을 관리
	void WaitForGpuComplete(); //"CPU와 GPU가 서로 맞춰서 일하게 만드는 함수" GPU 명령 큐가 작업을 다 끝냈는지 확인 후 GPU가 끝날 때까지 기다리는 코드
	//CPU와 GPU를 동기화하는 함수이다. 

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM //void OnProcessingMouseMessage(메세지를 받을 윈도우의 핸들, 발생한 메세지 종류, 마우스 버튼 상태, 마우스 커서 위치 정보)
		lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam,
		LPARAM lParam);
	//윈도우의 메시지(키보드, 마우스 입력)를 처리하는 함수이다. 

	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);

	public:
		//플레이어 객체에 대한 포인터이다.
		CPlayer* m_pPlayer = NULL;
		//마지막으로 마우스 버튼을 클릭할 때의 마우스 커서의 위치이다.
		POINT m_ptOldCursorPos;

		CGameObject* m_pSelectedObject = NULL;
};