#include "stdafx.h"
#include "3DGP_variableStage2.h"
#include "Scene.h"
#include "GraphicsPipeline.h"
#include "Stage2.h"

Stage2::Stage2(CPlayer* pPlayer) : CScene(pPlayer)
{
	m_pPlayer = pPlayer;
}

Stage2::~Stage2()
{
}

void Stage2::BuildObjects()
{
	CExplosiveObject::PrepareExplosion();

	CMesh* pEnemyTank = new CEnemyTankMesh();
	CMesh* pCube = new CCubeMesh();
	CMesh* pText = new CTextMesh(1.0f, 1.0f, 1.0f, 1, RGB(0, 0, 0), win, winLocationX, winLocationY);

	m_nObjects = count = 16;
	m_ppObjects = new CGameObject * [m_nObjects];

	for (int i = 0; i < 10; ++i) {
		m_ppObjects[i] = new CExplosiveObject();
		m_ppObjects[i]->SetMesh(pEnemyTank);
		m_ppObjects[i]->SetColor(RGB(0, 0, 0));

		float x = (rand() % 1001) - 500.0f; 
		float z = (rand() % 1001) - 500.0f;
		m_ppObjects[i]->SetPosition(x, 0.0f, z);

		float yaw = static_cast<float>(rand() % 360);
		XMMATRIX xmRotation = XMMatrixRotationY(XMConvertToRadians(yaw));

		XMMATRIX fixPitch = XMMatrixRotationX(XMConvertToRadians(90.0f));
		xmRotation = fixPitch * xmRotation;

		XMFLOAT4X4 xmfRotation;
		XMStoreFloat4x4(&xmfRotation, xmRotation);
		m_ppObjects[i]->SetRotationTransform(&xmfRotation);

		m_ppObjects[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppObjects[i]->SetRotationSpeed(0.0f);
		m_ppObjects[i]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_ppObjects[i]->SetMovingSpeed(0.0f);
	}

	for (int i = 10; i < 15; ++i) {
		m_ppObjects[i] = new CExplosiveObject();
		m_ppObjects[i]->SetMesh(pCube);
		m_ppObjects[i]->SetColor(RGB(0, 0, 0));

		float x = (rand() % 1001) - 500.0f; 
		float z = (rand() % 1001) - 500.0f;
		m_ppObjects[i]->SetPosition(x, 0.0f, z);

		float yaw = static_cast<float>(rand() % 360);
		XMMATRIX xmRotation = XMMatrixRotationY(XMConvertToRadians(yaw));

		XMMATRIX fixPitch = XMMatrixRotationX(XMConvertToRadians(90.0f));
		xmRotation = fixPitch * xmRotation;

		XMFLOAT4X4 xmfRotation;
		XMStoreFloat4x4(&xmfRotation, xmRotation);
		m_ppObjects[i]->SetRotationTransform(&xmfRotation);

		m_ppObjects[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppObjects[i]->SetRotationSpeed(0.0f);
		m_ppObjects[i]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_ppObjects[i]->SetMovingSpeed(0.0f);
	}

	m_ppObjects[15] = new CExplosiveObject();
	m_ppObjects[15]->SetMesh(pText);

	m_ppObjects[15]->SetColor(RGB(0, 0, 0));

	m_ppObjects[15]->SetPosition(0.0f, 0.0f, 0.0f);

	m_ppObjects[15]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppObjects[15]->SetRotationSpeed(0.0f);
	m_ppObjects[15]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_ppObjects[15]->SetMovingSpeed(0.0f);

	m_ppObjects[15]->SetActive(false);

#ifdef _WITH_DRAW_AXIS
	m_pWorldAxis = new CGameObject();
	CAxisMesh* pAxisMesh = new CAxisMesh(0.5f, 0.5f, 0.5f);
	m_pWorldAxis->SetMesh(pAxisMesh);
#endif
}

void Stage2::ReleaseObjects()
{
	if (CExplosiveObject::m_pExplosionMesh) CExplosiveObject::m_pExplosionMesh->Release();

	for (int i = 0; i < m_nObjects; i++) if (m_ppObjects[i]) delete m_ppObjects[i];
	if (m_ppObjects) delete[] m_ppObjects;


#ifdef _WITH_DRAW_AXIS
	if (m_pWorldAxis) delete m_pWorldAxis;
#endif
}

void Stage2::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (!m_pPlayer) return;

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		SetCapture(hWnd);
		m_pPlayer->m_bIsLButtonDown = true;
		break;

	case WM_LBUTTONUP:
		ReleaseCapture();
		m_pPlayer->m_bIsLButtonDown = false;
		break;
	}
}

void Stage2::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W':
			m_ppObjects[15]->SetActive(true);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void Stage2::Animate(float fElapsedTime)
{

	for (int i = 0; i < m_nObjects; i++) {
		m_ppObjects[i]->Animate(fElapsedTime);
		if (m_ppObjects[i]->getActive() && i < 10)
			++count;
	}

	if (count < 10) count = 0;
	else if(count == 10) m_ppObjects[15]->SetActive(true);
		

	CheckObjectByBulletCollisions();

	CScene::Animate(fElapsedTime);
	if (!m_pPlayer) return;


	if (m_pPlayer->m_bIsLButtonDown && GetCapture())
	{
		SetCursor(NULL); // 커서 숨김 (원하면 유지)

		POINT ptCursorPos;
		GetCursorPos(&ptCursorPos);

		// 화면 좌표 → 클라이언트 좌표
		ScreenToClient(m_hWnd, &ptCursorPos);

		float cxMouseDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		float cyMouseDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;

		SetCursorPos(ptCursorPos.x, ptCursorPos.y);

		if (cxMouseDelta != 0.0f || cyMouseDelta != 0.0f)
		{
			CPlayer* pPlayer = m_pPlayer;

			pPlayer->m_fCameraYaw -= cxMouseDelta;

			// pitch 고정값 설정
			pPlayer->m_fCameraPitch = 10.0f;

			if (!pPlayer->m_bOrbitMode) {
				pPlayer->Rotate(0.0f, cxMouseDelta, 0.0f);
			}
		}

		// 여기에서 매 프레임마다 기준 위치 갱신
		m_ptOldCursorPos = ptCursorPos;
	}

	m_pPlayer->Animate(fElapsedTime);
}


