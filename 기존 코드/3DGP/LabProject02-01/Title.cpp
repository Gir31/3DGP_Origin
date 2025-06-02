#include "stdafx.h"
#include "3DGP_variableTitle.h"
#include "Scene.h"
#include "GraphicsPipeline.h"
#include "Title.h"

Title::Title(CPlayer* pPlayer) : CScene(pPlayer)
{
	m_pPlayer = pPlayer;
}

Title::~Title()
{
}

void Title::BuildObjects()
{
	CExplosiveObject::PrepareExplosion();

	CTextMesh* pTitleMesh = new CTextMesh(1.0f, 1.0f, 1.0f, 0, RGB(0, 0, 0), title, titleLocationX, titleLocationY);
	CTextMesh* pNameMesh = new CTextMesh(1.0f, 1.0f, 1.0f, 1, RGB(0, 0, 0), name, nameLocationX, nameLocationY);

	m_nObjects = 2;
	m_ppObjects = new CGameObject * [m_nObjects];

	m_ppObjects[0] = new CExplosiveObject();
	m_ppObjects[0]->SetMesh(pNameMesh);

	m_ppObjects[0]->SetColor(RGB(0, 0, 0));

	m_ppObjects[0]->SetPosition(0.0f, 0.0f, 50.f);

	m_ppObjects[0]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppObjects[0]->SetRotationSpeed(90.0f);
	m_ppObjects[0]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_ppObjects[0]->SetMovingSpeed(0.0f);

	pNameMesh->Release();

	m_ppObjects[1] = new CExplosiveObject();
	m_ppObjects[1]->SetMesh(pTitleMesh);

	m_ppObjects[1]->SetColor(RGB(0, 0, 0));

	m_ppObjects[1]->SetPosition(0.0f, 10.0f, 50.0f);

	m_ppObjects[1]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
	m_ppObjects[1]->SetRotationSpeed(90.0f);
	m_ppObjects[1]->SetMovingDirection(XMFLOAT3(0.0f, 0.0f, 0.0f));
	m_ppObjects[1]->SetMovingSpeed(0.0f);

	pTitleMesh->Release();

	if (m_pPlayer)
	{
		m_pPlayer->m_fCameraYaw = 0.0f;
		m_pPlayer->m_fCameraPitch = 0.0f;

		m_pPlayer->m_pCamera->Update(m_pPlayer, m_pPlayer->m_xmf3Position, 0.0f);
		m_pPlayer->m_pCamera->GenerateViewMatrix();
	}

#ifdef _WITH_DRAW_AXIS
	m_pWorldAxis = new CGameObject();
	CAxisMesh* pAxisMesh = new CAxisMesh(0.5f, 0.5f, 0.5f);
	m_pWorldAxis->SetMesh(pAxisMesh);
#endif
}

void Title::ReleaseObjects()
{
	if (CExplosiveObject::m_pExplosionMesh) CExplosiveObject::m_pExplosionMesh->Release();

	for (int i = 0; i < m_nObjects; i++) if (m_ppObjects[i]) delete m_ppObjects[i];
	if (m_ppObjects) delete[] m_ppObjects;

#ifdef _WITH_DRAW_AXIS
	if (m_pWorldAxis) delete m_pWorldAxis;
#endif
}

void Title::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void Title::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		{
			CExplosiveObject* pExplosiveObject = (CExplosiveObject*)m_ppObjects[int(wParam - '1')];
			pExplosiveObject->m_bBlowingUp = true;
			break;
		}
		case 'A':
			for (int i = 0; i < m_nObjects; i++)
			{
				CExplosiveObject* pExplosiveObject = (CExplosiveObject*)m_ppObjects[i];
				pExplosiveObject->m_bBlowingUp = true;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

void Title::Animate(float fElapsedTime)
{
	for (int i = 0; i < m_nObjects; i++) m_ppObjects[i]->Animate(fElapsedTime);

	//CheckObjectByObjectCollisions();

	//CheckObjectByBulletCollisions();
}
