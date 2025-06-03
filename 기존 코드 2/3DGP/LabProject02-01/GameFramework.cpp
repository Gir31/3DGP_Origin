//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"

void CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	::srand(timeGetTime());

	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	BuildFrameBuffer(); 

	BuildObjects();

	_tcscpy_s(m_pszFrameRate, _T("3DGPProject("));
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();


	if (m_hBitmapFrameBuffer) ::DeleteObject(m_hBitmapFrameBuffer);
	if (m_hDCFrameBuffer) ::DeleteDC(m_hDCFrameBuffer);
}

void CGameFramework::BuildFrameBuffer()
{
	::GetClientRect(m_hWnd, &m_rcClient);

	HDC hDC = ::GetDC(m_hWnd);

    m_hDCFrameBuffer = ::CreateCompatibleDC(hDC);
	m_hBitmapFrameBuffer = ::CreateCompatibleBitmap(hDC, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top);
    ::SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);

	::ReleaseDC(m_hWnd, hDC);
    ::SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
}

void CGameFramework::ClearFrameBuffer(DWORD dwColor)
{
	HPEN hPen = ::CreatePen(PS_SOLID, 0, dwColor);
	HPEN hOldPen = (HPEN)::SelectObject(m_hDCFrameBuffer, hPen);
	HBRUSH hBrush = ::CreateSolidBrush(dwColor);
	HBRUSH hOldBrush = (HBRUSH)::SelectObject(m_hDCFrameBuffer, hBrush);
	::Rectangle(m_hDCFrameBuffer, m_rcClient.left, m_rcClient.top, m_rcClient.right, m_rcClient.bottom);
	::SelectObject(m_hDCFrameBuffer, hOldBrush);
	::SelectObject(m_hDCFrameBuffer, hOldPen);
	::DeleteObject(hPen);
	::DeleteObject(hBrush);
}

void CGameFramework::PresentFrameBuffer()
{    
    HDC hDC = ::GetDC(m_hWnd);
    ::BitBlt(hDC, m_rcClient.left, m_rcClient.top, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top, m_hDCFrameBuffer, m_rcClient.left, m_rcClient.top, SRCCOPY);
    ::ReleaseDC(m_hWnd, hDC);
}

void CGameFramework::BuildObjects()
{
	CCamera* pCamera = new CCamera();
	pCamera->SetViewport(0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);
	pCamera->GeneratePerspectiveProjectionMatrix(1.01f, 500.0f, 60.0f);
	pCamera->SetFOVAngle(60.0f);

	pCamera->GenerateOrthographicProjectionMatrix(1.01f, 50.0f, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT);

	CCartMesh* pCartMesh = new CCartMesh();
	CCursorMesh* pCursor = new CCursorMesh();
	CTankBodyMesh* pTankBodyMesh = new CTankBodyMesh();
	CTankTurretMesh* pTankTurretMesh = new CTankTurretMesh();

	m_pPlayer[0] = new CCursor();
	m_pPlayer[0]->SetPosition(0.0f, 0.0f, 0.0f);
	m_pPlayer[0]->SetMesh(pCursor);
	m_pPlayer[0]->SetColor(RGB(0, 0, 255));
	m_pPlayer[0]->SetCamera(pCamera);
	m_pPlayer[0]->SetCameraOffset(XMFLOAT3(0.0f, 5.0f, -15.0f));

	m_pPlayer[1] = new CCursor();
	m_pPlayer[1]->SetPosition(0.0f, 0.0f, 0.0f);
	m_pPlayer[1]->SetMesh(pCursor);
	m_pPlayer[1]->SetColor(RGB(0, 0, 255));
	m_pPlayer[1]->SetCamera(pCamera);
	m_pPlayer[1]->SetCameraOffset(XMFLOAT3(0.0f, 5.0f, -15.0f));

	m_pPlayer[2] = new CCart();
	m_pPlayer[2]->SetPosition(0.0f, 0.0f, 0.0f);
	m_pPlayer[2]->SetMesh(pCartMesh);
	m_pPlayer[2]->SetColor(RGB(0, 0, 255));
	m_pPlayer[2]->SetCamera(pCamera);
	m_pPlayer[2]->SetCameraOffset(XMFLOAT3(0.0f, 5.0f, -15.0f));

	CTank* pTankPlayer = new CTank(pCamera, pTankTurretMesh);
	pTankPlayer->SetPosition(0.f, 0.f, 0.f);
	pTankPlayer->SetMesh(pTankBodyMesh);
	pTankPlayer->SetColor(RGB(0, 0, 255));
	pTankPlayer->SetCamera(pCamera);
	pTankPlayer->SetCameraOffset(XMFLOAT3(0.0f, 5.0f, -15.0f));

	m_pPlayer[3] = pTankPlayer;

	manager = new StageManager(new Title(m_pPlayer[0]), new Menu(m_pPlayer[1]), new Stage1(m_pPlayer[2]), new Stage2(m_pPlayer[3]));
	manager->setPlayer(m_pPlayer[0], 0);
	manager->setPlayer(m_pPlayer[1], 1);
	manager->setPlayer(m_pPlayer[2], 2);
	manager->setPlayer(m_pPlayer[3], 3);
	manager->buildStage();
}

void CGameFramework::ReleaseObjects()
{
	if (manager->getCurrStage())
	{
		manager->getCurrStage()->ReleaseObjects();
		delete manager->getCurrStage();
	}

	if (m_pPlayer[manager->getCurrLevel()]) delete m_pPlayer[manager->getCurrLevel()];
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{

	if (manager->getCurrStage()) {
		manager->getCurrStage()->OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
	}

	switch (nMessageID)
	{
	case WM_RBUTTONDOWN:
	{
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);

		int currLevel = manager->getCurrLevel();
		CPlayer* p = manager->getPlayer(currLevel);

		m_pAutoAttackObject = manager->getCurrStage()->
			PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), p->m_pCamera);

		break;
	}
	case WM_LBUTTONDOWN:
	{
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);

		CExplosiveObject* pExplosiveObject =
			(CExplosiveObject*)manager->getCurrStage()->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pPlayer[manager->getCurrLevel()]->m_pCamera);
		if (manager->getCurrLevel() == 0 || manager->getCurrLevel() == 1) {
			if (pExplosiveObject) {
				pExplosiveObject->m_bBlowingUp = true;
				if (pExplosiveObject->getTargetStage() != manager->getCurrLevel()) {
					manager->setReady(true);
					manager->setNextLevel(pExplosiveObject->getTargetStage());
				}
			}
		}

		break;
	}
	case WM_LBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
	{
		break;
	}
	default:
		break;
	}
}
void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (manager->getCurrStage()) manager->getCurrStage()->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);

	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_ESCAPE:
			if (manager->getCurrLevel() == 2 || manager->getCurrLevel() == 3) {
				manager->setChange(true);
				manager->changeStage(1);
			}
			break;
		case VK_RETURN:
			break;
		case VK_CONTROL:
			if (manager->getCurrLevel() == 3) {
				((CTank*)m_pPlayer[manager->getCurrLevel()])->FireBullet(m_pLockedObject);
				m_pLockedObject = NULL;
			}
			break;
		case 'C': 
			m_pPlayer[manager->getCurrLevel()]->m_bOrbitMode = !m_pPlayer[manager->getCurrLevel()]->m_bOrbitMode;

			if (m_pPlayer[manager->getCurrLevel()]->m_bOrbitMode)
			{
				m_pPlayer[manager->getCurrLevel()]->m_fSavedYaw = m_pPlayer[manager->getCurrLevel()]->m_fCameraYaw;
				m_pPlayer[manager->getCurrLevel()]->m_fSavedPitch = m_pPlayer[manager->getCurrLevel()]->m_fCameraPitch;
			}
			else
			{
				m_pPlayer[manager->getCurrLevel()]->m_fCameraYaw = m_pPlayer[manager->getCurrLevel()]->m_fSavedYaw;
				m_pPlayer[manager->getCurrLevel()]->m_fCameraPitch = std::clamp(m_pPlayer[manager->getCurrLevel()]->m_fSavedPitch, -89.0f, 89.0f);

				float yawRad = XMConvertToRadians(m_pPlayer[manager->getCurrLevel()]->m_fCameraYaw);

				m_pPlayer[manager->getCurrLevel()]->m_xmf3Look.x = -sinf(yawRad);
				m_pPlayer[manager->getCurrLevel()]->m_xmf3Look.y = 0.0f; 
				m_pPlayer[manager->getCurrLevel()]->m_xmf3Look.z = cosf(yawRad);
				m_pPlayer[manager->getCurrLevel()]->m_xmf3Look = Vector3::Normalize(m_pPlayer[manager->getCurrLevel()]->m_xmf3Look);

				m_pPlayer[manager->getCurrLevel()]->m_xmf3Right = Vector3::CrossProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), m_pPlayer[manager->getCurrLevel()]->m_xmf3Look);
				m_pPlayer[manager->getCurrLevel()]->m_xmf3Right = Vector3::Normalize(m_pPlayer[manager->getCurrLevel()]->m_xmf3Right);

				m_pPlayer[manager->getCurrLevel()]->m_pCamera->Update(m_pPlayer[manager->getCurrLevel()], m_pPlayer[manager->getCurrLevel()]->m_xmf3Position, 0.0f);
				m_pPlayer[manager->getCurrLevel()]->m_pCamera->GenerateViewMatrix();
			}

			break;
		case 'N':
			if (manager->getCurrLevel() == 2) {
				manager->setChange(true);
				manager->changeStage(3);
			}
			break;
		case 'A':
			if (m_pAutoAttackObject)
				m_pLockedObject = m_pAutoAttackObject;
			break;
		case 'S':
			if(manager->getCurrLevel() == 3)
				m_pPlayer[manager->getCurrLevel()]->getMesh()->beSheild = !m_pPlayer[manager->getCurrLevel()]->getMesh()->beSheild;
			break;
		case 'W':
			break;
		default:
			manager->getCurrStage()->OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CALLBACK CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_ACTIVATE:
	{
		if (LOWORD(wParam) == WA_INACTIVE)
			m_GameTimer.Stop();
		else
			m_GameTimer.Start();
		break;
	}
	case WM_SIZE:
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return(0);
}

void CGameFramework::ProcessInput()
{
	static UCHAR pKeyBuffer[256];
	if (GetKeyboardState(pKeyBuffer))
	{
		DWORD dwDirection = 0;
		if (pKeyBuffer[VK_UP] & 0xF0) dwDirection |= DIR_FORWARD;
		if (pKeyBuffer[VK_DOWN] & 0xF0) dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer[VK_LEFT] & 0xF0) dwDirection |= DIR_LEFT;
		if (pKeyBuffer[VK_RIGHT] & 0xF0) dwDirection |= DIR_RIGHT;
		if (pKeyBuffer[VK_PRIOR] & 0xF0) dwDirection |= DIR_UP;
		if (pKeyBuffer[VK_NEXT] & 0xF0) dwDirection |= DIR_DOWN;

		if (dwDirection) m_pPlayer[manager->getCurrLevel()]->Move(dwDirection, 0.15f);
	}
	m_pPlayer[manager->getCurrLevel()]->Update(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::AnimateObjects()
{
	float fTimeElapsed = m_GameTimer.GetTimeElapsed();

	if (m_pPlayer[manager->getCurrLevel()]) m_pPlayer[manager->getCurrLevel()]->Animate(fTimeElapsed);
	if (manager->getCurrStage()) manager->getCurrStage()->Animate(fTimeElapsed);
	if (manager->getReady()) manager->waitTime(fTimeElapsed);

	if (manager->getCurrLevel() == 2 && ((CCart*)manager->getPlayer(2))->getFlag()) {
		manager->setChange(true);
		manager->changeStage(3);
	}
}

void CGameFramework::FrameAdvance()
{    
	m_GameTimer.Tick(60.0f);

	ProcessInput();

	AnimateObjects();

    ClearFrameBuffer(RGB(255, 255, 255));

	CCamera* pCamera = m_pPlayer[manager->getCurrLevel()]->GetCamera();
	if (manager->getCurrStage()) manager->getCurrStage()->Render(m_hDCFrameBuffer, pCamera);

	PresentFrameBuffer();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}