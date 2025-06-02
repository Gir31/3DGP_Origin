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


	BuildObjects();

	_tcscpy_s(m_pszFrameRate, _T("3DGPProject("));
}

void CGameFramework::OnDestroy()
{
	ReleaseObjects();
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

// [추가]
void CGameFramework::CreateDirect3DDevice()
{
	// DXGI 팩토리 생성
	HRESULT hr = CreateDXGIFactory1(IID_PPV_ARGS(&m_pdxgiFactory));
	if (FAILED(hr)) {
		::MessageBox(NULL, L"CreateDXGIFactory1() failed.", L"Error", MB_OK);
		exit(-1);
	}

	// 하드웨어 어댑터 선택 (기본 GPU 사용)
	IDXGIAdapter1* pAdapter = nullptr;
	m_pdxgiFactory->EnumAdapters1(0, &pAdapter);

	// D3D12 디바이스 생성
	hr = D3D12CreateDevice(pAdapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_pd3dDevice));
	if (FAILED(hr)) {
		::MessageBox(NULL, L"D3D12CreateDevice() failed.", L"Error", MB_OK);
		exit(-1);
	}

	if (pAdapter) pAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	// 1. Command Queue 생성
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT; // 일반적인 그래픽스 명령용
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr = m_pd3dDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_pd3dCommandQueue));
	if (FAILED(hr)) {
		::MessageBox(NULL, L"CreateCommandQueue() failed.", L"Error", MB_OK);
		exit(-1);
	}

	// 2. Command Allocator 생성
	hr = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pd3dCommandAllocator));
	if (FAILED(hr)) {
		::MessageBox(NULL, L"CreateCommandAllocator() failed.", L"Error", MB_OK);
		exit(-1);
	}

	// 3. Command List 생성
	hr = m_pd3dDevice->CreateCommandList(
		0, // 노드 마스크 (싱글 GPU 시스템은 0)
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pd3dCommandAllocator,
		nullptr, // 초기 파이프라인 상태 객체 없음
		IID_PPV_ARGS(&m_pd3dCommandList)
	);
	if (FAILED(hr)) {
		::MessageBox(NULL, L"CreateCommandList() failed.", L"Error", MB_OK);
		exit(-1);
	}

	// 커맨드 리스트는 처음 생성 시 열린(open) 상태 → 닫기(close)
	m_pd3dCommandList->Close();
}
