#pragma once

#include "GameObject.h"
#include "Camera.h"
#include "Player.h"


class CScene
{
public:
	CScene(CPlayer* pPlayer);
	virtual ~CScene();

	//////////////////////////////////////////
	// ������Ʈ �� �÷��̾�
protected:
	int							m_nObjects = 0;
	CGameObject** m_ppObjects = NULL;
	CPlayer* m_pPlayer = NULL;

#ifdef _WITH_DRAW_AXIS
	CGameObject* m_pWorldAxis = NULL;
#endif

	//////////////////////////////////////////
	// �׷��Ƚ� ����������
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	ID3D12PipelineState* m_pd3dPipelineState = NULL;

	//////////////////////////////////////////
	// ������Ʈ ����
public:
	virtual void BuildObjects();                          // WinAPI��
	virtual void ReleaseObjects();

	virtual void BuildObjects(ID3D12Device* pd3dDevice);  // D3D12��
	virtual void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice);

	//////////////////////////////////////////
	// ������ �� �ִϸ��̼�
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);				 // WinAPI��
	virtual void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);          // D3D12��
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);                 // D3D12��

	//////////////////////////////////////////
	// �浹 �� �Է� ó��
	void CheckObjectByObjectCollisions();
	void CheckObjectByBulletCollisions();

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//////////////////////////////////////////
	// ��Ÿ
	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);
	virtual bool IsCameraMovable() const { return true; }

	bool ProcessInput();

};