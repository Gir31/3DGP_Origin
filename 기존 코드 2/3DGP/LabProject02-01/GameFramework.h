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
	// ������ ����
	HINSTANCE						m_hInstance = nullptr;
	HWND							m_hWnd = nullptr;
	int								m_nWndClientWidth;
	int								m_nWndClientHeight;

	//////////////////////////////////////////
	// Direct3D 12 �⺻ ����
	IDXGIFactory4*					m_pdxgiFactory;							// DXGI ���丮
	IDXGISwapChain3*				m_pdxgiSwapChain;						// ���� ü��
	ID3D12Device*					m_pd3dDevice;							// ����̽�

	//////////////////////////////////////////
	// MSAA ����
	bool							m_bMsaa4xEnable = false;				// 4x MSAA ��� ����
	UINT							m_nMsaa4xQualityLevels = 0;				// MSAA ǰ�� ����

	//////////////////////////////////////////
	// ���� ü�� ����
	static const UINT				m_nSwapChainBuffers = 2;				// ���� ü�� �ĸ� ���� ����
	UINT							m_nSwapChainBufferIndex;				// ���� ��� ���� ���� �ε���
	ID3D12Resource*					m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];	// ���� Ÿ�� ����
	ID3D12DescriptorHeap*			m_pd3dRtvDescriptorHeap;				// RTV ������ ��
	UINT							m_nRtvDescriptorIncrementSize;			// RTV ������ ���� ũ��

	//////////////////////////////////////////
	// ����/���ٽ� ����
	ID3D12Resource*					m_pd3dDepthStencilBuffer;				// ���� ���ٽ� ����
	ID3D12DescriptorHeap*			m_pd3dDsvDescriptorHeap;				// DSV ������ ��
	UINT							m_nDsvDescriptorIncrementSize;			// DSV ������ ���� ũ��

	//////////////////////////////////////////
	// Ŀ�ǵ� ��ü
	ID3D12CommandQueue*				m_pd3dCommandQueue;
	ID3D12CommandAllocator*			m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList*		m_pd3dCommandList;

	//////////////////////////////////////////
	// ���������� ���� �� ����ȭ
	ID3D12PipelineState*			m_pd3dPipelineState;					// ���������� ���� ��ü
	ID3D12Fence*					m_pd3dFence;							// �潺 ��ü
	UINT64							m_nFenceValue[m_nSwapChainBuffers];		// �潺 ��
	HANDLE							m_hFenceEvent;							// �潺 �̺�Ʈ �ڵ�

	//////////////////////////////////////////
	// ����Ʈ �� ��ũ��
	D3D12_VIEWPORT					m_d3dViewport = {};
	D3D12_RECT						m_d3dScissorRect = {};

	//////////////////////////////////////////
	// ���� �÷��� ����
	std::array<CPlayer*, 4>			m_pPlayer = {};
	CGameObject*					m_pAutoAttackObject = nullptr;
	CGameObject*					m_pLockedObject = nullptr;
	StageManager*					manager = nullptr;

	//////////////////////////////////////////
	// �ý��� ��ƿ��Ƽ
	CGameTimer						m_GameTimer;
	POINT							m_ptOldCursorPos = {};
	_TCHAR							m_pszFrameRate[50] = { 0 };

	//////////////////////////////////////////
	// ���� ����
	bool							m_bActive = true;
	bool							m_bShieldActive = false;
public:
	// ������ / �Ҹ���
	CGameFramework();
	~CGameFramework();

	// �ʱ�ȭ / ����
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	///////////////////////////////////////////////
	// Direct3D 12 �ʱ�ȭ ����
	void CreateDirect3DDevice();                  // ����̽� ����
	void CreateCommandQueueAndList();             // ��� ť, �Ҵ���, ����Ʈ ����
	void CreateSwapChain();                       // ���� ü�� ����
	void CreateRtvAndDsvDescriptorHeaps();        // RTV/DSV ������ �� ����
	void CreateRenderTargetViews();               // ���� Ÿ�� �� ����
	void CreateDepthStencilView();                // ����-���ٽ� �� ����

	///////////////////////////////////////////////
	// ������ ���� �� ������
	void BuildFrameBuffer();                      // ������ ���� ����
	void ClearFrameBuffer(DWORD dwColor);         // ȭ�� �ʱ�ȭ
	void PresentFrameBuffer();                    // ȭ�� ��� (Present)

	///////////////////////////////////////////////
	// ���� ������Ʈ ����
	void BuildObjects();                          // ���� ������Ʈ ����
	void ReleaseObjects();                        // ���� ������Ʈ ����

	///////////////////////////////////////////////
	// ���� ���� ó��
	void ProcessInput();                          // �Է� ó��
	void AnimateObjects();                        // �ִϸ��̼� �� ���� ����
	void FrameAdvance();                          // ������ ��ȯ ó��

	///////////////////////////////////////////////
	// GPU ����ȭ
	void WaitForGpuComplete();                    // GPU �۾� �Ϸ� ���
	void MoveToNextFrame();

	///////////////////////////////////////////////
	// �Է� �޽��� ó��
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	///////////////////////////////////////////////
	// ��Ÿ
	void SetActive(bool bActive) { m_bActive = bActive; }
};

