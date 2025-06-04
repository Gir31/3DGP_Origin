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
	// 오브젝트 및 플레이어
protected:
	int							m_nObjects = 0;
	CGameObject** m_ppObjects = NULL;
	CPlayer* m_pPlayer = NULL;

#ifdef _WITH_DRAW_AXIS
	CGameObject* m_pWorldAxis = NULL;
#endif

	//////////////////////////////////////////
	// 그래픽스 파이프라인
	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
	ID3D12PipelineState* m_pd3dPipelineState = NULL;

	//////////////////////////////////////////
	// 오브젝트 관리
public:
	virtual void BuildObjects();                          // WinAPI용
	virtual void ReleaseObjects();

	virtual void BuildObjects(ID3D12Device* pd3dDevice);  // D3D12용
	virtual void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	virtual void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice);

	//////////////////////////////////////////
	// 렌더링 및 애니메이션
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);				 // WinAPI용
	virtual void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);          // D3D12용
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);                 // D3D12용

	//////////////////////////////////////////
	// 충돌 및 입력 처리
	void CheckObjectByObjectCollisions();
	void CheckObjectByBulletCollisions();

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	//////////////////////////////////////////
	// 기타
	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);
	virtual bool IsCameraMovable() const { return true; }

	bool ProcessInput();

};