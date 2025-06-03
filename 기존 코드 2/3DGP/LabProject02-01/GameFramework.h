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
	// ������ �ڵ� �� ũ�� ����
	HINSTANCE m_hInstance;                              // ���ø����̼� �ν��Ͻ� �ڵ�
	HWND      m_hWnd;                                   // ������ �ڵ�
	int       m_nWndClientWidth;                        // Ŭ���̾�Ʈ ���� �ʺ�
	int       m_nWndClientHeight;                       // Ŭ���̾�Ʈ ���� ����

	// Direct3D 12 ��ġ �� DXGI ���丮
	IDXGIFactory4* m_pdxgiFactory;						// DXGI ���丮 (����̽� ������ �ʿ�)
	IDXGISwapChain3* m_pdxgiSwapChain;					// ����ü�� (���� ��ȯ��)
	ID3D12Device* m_pd3dDevice;							// D3D12 ����̽� (GPU �ڿ� ���� ���)

	// MSAA ����
	bool m_bMsaa4xEnable = false;                       // 4x MSAA ��� ����
	UINT m_nMsaa4xQualityLevels = 0;                    // MSAA ǰ�� ���� �� (���� ����)

	// ����ü�� �� ����Ÿ�� ����
	static const UINT m_nSwapChainBuffers = 2;          // ����ü�� ���� �� (���� ���۸�)
	UINT m_nSwapChainBufferIndex;                       // ���� ��� ���� ����� �ε���

	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers]; // ����Ÿ�ٿ� ����� ���ҽ�
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;      // RTV(����Ÿ�ٺ�) ��ũ���� ��
	UINT m_nRtvDescriptorIncrementSize;                 // RTV ��ũ���� ���� ũ��

	// ���� ���ٽ� ����
	ID3D12Resource* m_pd3dDepthStencilBuffer;			// ����/���ٽǿ� ���� ���ҽ�
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;      // DSV(����/���ٽ� ��) ��ũ���� ��
	UINT m_nDsvDescriptorIncrementSize;                 // DSV ��ũ���� ���� ũ��

	// Ŀ�ǵ� ť, ����Ʈ, �Ҵ���
	ID3D12CommandAllocator* m_pd3dCommandAllocator;		// ��� ����Ʈ �޸� ������
	ID3D12CommandQueue* m_pd3dCommandQueue;				// GPU ��� ť
	ID3D12GraphicsCommandList* m_pd3dCommandList;		// ��� ����Ʈ (�׸��� ��� ����)

	// ����ȭ ��ü (GPU-CPU ���� ���߱�)
	ID3D12Fence*	m_pd3dFence;						// GPU �۾� �Ϸ� ���� Ȯ�ο� �潺
	UINT64			m_nFenceValues[m_nSwapChainBuffers];// ���ۺ� �潺 ��
	HANDLE			m_hFenceEvent;                      // �潺 �̺�Ʈ �ڵ�

	//===== ���������� ���� =====
	ID3D12PipelineState* m_pd3dPipelineState;
	D3D12_VIEWPORT m_d3dViewport;
	D3D12_RECT m_d3dScissorRect;

	//===== [�߰�] ��Ʈ �ñ״�ó �� CBV/SRV �� =====
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = nullptr;         // ��Ʈ �ñ״�ó
	ID3D12DescriptorHeap* m_pd3dCbvSrvDescriptorHeap = nullptr;         // CBV/SRV ��ũ���� ��
	UINT m_nCbvSrvDescriptorIncrementSize = 0;                          // CBV/SRV ��ũ���� ������


#if defined(_DEBUG)
	ID3D12Debug* m_pd3dDebugController;					// D3D ����� ���̾�
#endif

	// ���� ��ü
	std::array<CPlayer*, 4> m_pPlayer;                  // �ִ� 4���� �÷��̾� ������ �迭
	CGameObject* m_pAutoAttackObject = NULL;            // �ڵ� ���� ���
	CGameObject* m_pLockedObject = NULL;                // ���� ������ �� ��ü

	StageManager* manager = NULL;                       // ���� ��������/���� �����ϴ� �Ŵ��� Ŭ����

	CGameTimer m_GameTimer;                             // ��ü ������ Ÿ�̸�

	POINT m_ptOldCursorPos;                             // ���� ������ ���� ���콺 Ŀ�� ��ġ
	_TCHAR m_pszFrameRate[50];                          // ������ �ӵ� ���ڿ� ����

	bool m_bShieldActive = false;                       // �÷��̾� ���� Ȱ�� ���� ����


public:
	// �ý��� �ʱ�ȭ �� ����
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);   // ��ü �ý��� ����
	void OnDestroy();                                    // ��ü �ý��� ���� �� ���ҽ� ����

	// Direct3D 12 �ʱ�ȭ ����
	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
	void CreateRtvAndDsvDescriptorHeaps();
	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void ChangeSwapChainState();                         // Ǯ��ũ�� ��� ��

	//===== D3D12 �ʼ� ���� �Լ� =====
	void CreateCbvSrvDescriptorHeaps();      // CBV/SRV ��ũ���� �� ����
	void CreateGraphicsRootSignature();      // ��Ʈ �ñ״�ó ����

	// ���� ���� ����
	void BuildObjects();                                 // ��/������Ʈ ����
	void ReleaseObjects();                               // ��/������Ʈ ����

	// ���� ���� �� ������Ʈ
	void ProcessInput();                                 // �Է� ó��
	void AnimateObjects();                               // ������Ʈ �ִϸ��̼� (��ȯ ��)
	void FrameAdvance();                                 // �� ������ ó�� (������Ʈ + ������)

	// ���̴� ���� ó��
	virtual void CreateShaderVariables();                // CBV �� ����
	virtual void ReleaseShaderVariables();               // CBV ����
	virtual void UpdateShaderVariables();                // CBV �� �� ������Ʈ

	// GPU ����ȭ ����
	void WaitForGpuComplete();                           // GPU ��� �Ϸ� ���
	void MoveToNextFrame();                              // ������ ��ȯ (���� ����۷�)

	// �Է� �޽��� ó��
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
};

