#pragma once

#include "GameObject.h"
#include "Camera.h"
#include "Player.h"


class CScene
{
public:
	CScene(CPlayer *pPlayer);
	virtual ~CScene();

protected:
	int							m_nObjects = 0;
	CGameObject					**m_ppObjects = NULL;

	CPlayer*					m_pPlayer = NULL;

#ifdef _WITH_DRAW_AXIS
	CGameObject*				m_pWorldAxis = NULL;
#endif

public:
	virtual void BuildObjects();
	virtual void ReleaseObjects();

	void CheckObjectByObjectCollisions();
	void CheckObjectByBulletCollisions();

	virtual void Animate(float fElapsedTime);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);


	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);

	virtual bool IsCameraMovable() const { return true; }
};