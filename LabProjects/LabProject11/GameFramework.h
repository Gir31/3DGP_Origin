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
	//������ ���� �����ӿ�ũ���� ����� Ÿ�̸��̴�.
	CGameTimer m_GameTimer;
	//������ ������ ����Ʈ�� �� �������� ĸ�ǿ� ����ϱ� ���� ���ڿ��̴�.
	_TCHAR m_pszFrameRate[50];

	HINSTANCE m_hInstance;
	HWND m_hWnd;
	int m_nWndClientWidth;
	int m_nWndClientHeight;

	//Direct3D�� �ü��(������)�� ���÷��� �ý����� �������ִ� �߰� ���� API
	IDXGIFactory4* m_pdxgiFactory;
	//DXGI ���丮 �������̽��� ���� �������̴�. 

	//������ ����� �����ִ� ���� ��ü �ý���
	IDXGISwapChain3* m_pdxgiSwapChain;
	//���� ü�� �������̽��� ���� �������̴�. �ַ� ���÷��̸� �����ϱ� ���Ͽ� �ʿ��ϴ�.

	//GPU�� ���� �� �� �ֵ��� ����� �غ��Ű�� �ٽ� �������̽�(GPU�� ������ ��ɾ ť�� �־��)
	ID3D12Device* m_pd3dDevice;
	//Direct3D ����̽� �������̽��� ���� �������̴�. �ַ� ���ҽ��� �����ϱ� ���Ͽ� �ʿ��ϴ�.

	//MSAA : ������ ����� **��� ����(jaggies)**�� ���̱� ���� ���
	bool m_bMsaa4xEnable = false;
	UINT m_nMsaa4xQualityLevels = 0;
	//MSAA ���� ���ø��� Ȱ��ȭ�ϰ� ���� ���ø� ������ �����Ѵ�.

	static const UINT m_nSwapChainBuffers = 2;
	//���� ü���� �ĸ� ������ �����̴�.

	UINT m_nSwapChainBufferIndex;
	//���� ���� ü���� �ĸ� ���� �ε����̴�.

	//����� ���� Ÿ�ٵ��� �����ϴ� �迭
	ID3D12Resource* m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap; //Rtv�� Descriptor�� �迭�� �����ϴ� �迭
	UINT m_nRtvDescriptorIncrementSize;
	//���� Ÿ�� ����, ������ �� �������̽� ������, ���� Ÿ�� ������ ������ ũ���̴�.
	//������(Descriptor) : GPU ���ҽ�(�ؽ�ó, ���� ��)�� ����ϴ� ����� ���� ������.
	//��, GPU�� "�� ���ҽ��� �̷��� ����ض�"��� ������ �� �ְ� �����ִ� ��(View) ���� ��.


	ID3D12Resource* m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;
	//����-���ٽ� ����, ������ �� �������̽� ������, ����-���ٽ� ������ ������ ũ���̴�.

	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12CommandAllocator* m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	//��� ť, ��� �Ҵ���, ��� ����Ʈ �������̽� �������̴�.

	//�������� �� �ʿ��� ��� ���������� ���� ��Ҹ� GPU�� �̸� �����صδ� ����
	ID3D12PipelineState* m_pd3dPipelineState;
	//�׷��Ƚ� ���������� ���� ��ü�� ���� �������̽� �������̴�.

	//GPU�� ��� ť(�׷��Ƚ� �۾�) ���� ���¸� �����ϰų� CPU�� GPU ���� ����ȭ�� �� �����.
	//"GPU�� ������� �ߴ��� ? �� ��� �� �ְ� ����
	ID3D12Fence* m_pd3dFence;
	//�潺�� ���� ���� ��. �׻� ���� ���ؼ� ����ȭ������
	UINT64 m_nFenceValues[m_nSwapChainBuffers];

	CScene* m_pScene;

	//CPU�� GPU�� �۾� �ϷḦ ��ٸ��� ���� ����ϴ� �̺�Ʈ �ڵ�
	HANDLE m_hFenceEvent;
	//�潺 �������̽� ������, �潺�� ��, �̺�Ʈ �ڵ��̴�.

	StageManager* manager = NULL;

	//����Ʈ�� ���� �簢���̴�. 
	ID3D12Resource* m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
public:
	CGameFramework();
	~CGameFramework();

	CCamera* m_pCamera = NULL;

	void MoveToNextFrame();

	void ChangeSwapChainState();

	//� ��ü(Ȥ�� ���ø����̼�)�� �ʱ�ȭ �۾��� ����ϴ� �Լ�. ��, ������ ������ �� �̸� �����صξ���ϴ� ���ҽ� �� �����͵�� �ʱ�ȭ���ְڴٴ� ���̴�.
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	//�����ӿ�ũ�� �ʱ�ȭ�ϴ� �Լ��̴�(�� �����찡 �����Ǹ� ȣ��ȴ�). 

	void OnDestroy();//������ ���ҽ��� �����ϴ� ��. ���� ���� �� ����
	void CreateSwapChain(); //����ü���� ����� �Լ�
	void CreateRtvAndDsvDescriptorHeaps(); //���� Ÿ��(RTV) & ���� ���ٽ�(DSV) ��� ��ũ���� �� ����.
	void CreateDirect3DDevice(); //ID3D12Device�� �����ؼ� GPU�� ����� �� �ִ� ��ü�� �����.
	void CreateCommandQueueAndList(); //GPU���� ����� ���� Ŀ�ǵ� ť�� Ŀ�ǵ� ����Ʈ�� ����� ��
	//���� ü��, ����̽�, ������ ��, ��� ť/�Ҵ���/����Ʈ�� �����ϴ� �Լ��̴�.     

	void CreateRenderTargetViews(); //GPU�� ���� �׷� �ִ� �뵵�� ������ �� �ֵ��� ��
	void CreateDepthStencilView();  //����/���ٽ� �׽�Ʈ �뵵�� �ؽ�ó ����
	//���� Ÿ�� ��� ����-���ٽ� �並 �����ϴ� �Լ��̴�. 

	void BuildObjects();
	void ReleaseObjects();
	//�������� �޽��� ���� ��ü�� �����ϰ� �Ҹ��ϴ� �Լ��̴�. 
   //�����ӿ�ũ�� �ٽ�(����� �Է�, �ִϸ��̼�, ������)�� �����ϴ� �Լ��̴�. 

	void ProcessInput(); //����� �Է� (Ű����, ���콺, ��Ʈ�ѷ� ��)�� ó���ϴ� �Լ�
	void AnimateObjects(); //���� �� ������Ʈ(�÷��̾�, ��, ��� ��)�� �ִϸ��̼� ����
	void FrameAdvance(); //�� �������� ��ü �帧�� ����
	void WaitForGpuComplete(); //"CPU�� GPU�� ���� ���缭 ���ϰ� ����� �Լ�" GPU ��� ť�� �۾��� �� ���´��� Ȯ�� �� GPU�� ���� ������ ��ٸ��� �ڵ�
	//CPU�� GPU�� ����ȭ�ϴ� �Լ��̴�. 

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM //void OnProcessingMouseMessage(�޼����� ���� �������� �ڵ�, �߻��� �޼��� ����, ���콺 ��ư ����, ���콺 Ŀ�� ��ġ ����)
		lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM
		lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam,
		LPARAM lParam);
	//�������� �޽���(Ű����, ���콺 �Է�)�� ó���ϴ� �Լ��̴�. 

	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);

	public:
		//�÷��̾� ��ü�� ���� �������̴�.
		CPlayer* m_pPlayer = NULL;
		//���������� ���콺 ��ư�� Ŭ���� ���� ���콺 Ŀ���� ��ġ�̴�.
		POINT m_ptOldCursorPos;

		CGameObject* m_pSelectedObject = NULL;
};