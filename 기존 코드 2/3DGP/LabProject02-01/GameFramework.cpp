//-----------------------------------------------------------------------------
// File: CGameFramework.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "GameFramework.h"

CGameFramework::CGameFramework()
{
	//////////////////////////////////////////
	// DXGI ���丮 �� ����̽�
	m_pdxgiFactory = nullptr;
	m_pdxgiSwapChain = nullptr;
	m_pd3dDevice = nullptr;

	//////////////////////////////////////////
	// Ŀ�ǵ� ť �� Ŀ�ǵ� ����Ʈ
	m_pd3dCommandQueue = nullptr;
	m_pd3dCommandAllocator = nullptr;
	m_pd3dCommandList = nullptr;
	m_pd3dPipelineState = nullptr;

	//////////////////////////////////////////
	// ���� ü�� ����
	m_nSwapChainBufferIndex = 0;
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		m_ppd3dRenderTargetBuffers[i] = nullptr;

	//////////////////////////////////////////
	// RTV (Render Target View)
	m_pd3dRtvDescriptorHeap = nullptr;
	m_nRtvDescriptorIncrementSize = 0;

	//////////////////////////////////////////
	// DSV (Depth Stencil View)
	m_pd3dDepthStencilBuffer = nullptr;
	m_pd3dDsvDescriptorHeap = nullptr;
	m_nDsvDescriptorIncrementSize = 0;

	//////////////////////////////////////////
	// ����ȭ ��ü
	m_pd3dFence = nullptr;
	for (int i = 0; i < m_nSwapChainBuffers; i++)
		m_nFenceValue[i] = 0;
	m_hFenceEvent = nullptr;

	//////////////////////////////////////////
	// ����Ʈ �� ���� �簢��
	m_d3dViewport = { 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT, 0.0f, 1.0f };
	m_d3dScissorRect = { 0, 0, FRAMEBUFFER_WIDTH, FRAMEBUFFER_HEIGHT };

	//////////////////////////////////////////
	// ������ ũ�� ����
	m_nWndClientWidth = FRAMEBUFFER_WIDTH;
	m_nWndClientHeight = FRAMEBUFFER_HEIGHT;

}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	// ���� �ʱ�ȭ
	::srand(timeGetTime());

	// �ν��Ͻ� �� ������ �ڵ� ����
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	////////////////////////////////////////////////////////
	// Direct3D 12 �ʱ�ȭ ����
	CreateDirect3DDevice();               // ����̽� ����
	CreateCommandQueueAndList();          // ��� ť, �Ҵ���, ����Ʈ ����
	CreateSwapChain();                    // ���� ü�� ����
	CreateRtvAndDsvDescriptorHeaps();     // RTV/DSV ������ �� ����
	CreateRenderTargetViews();            // ���� Ÿ�� �� ����
	CreateDepthStencilView();             // ����/���ٽ� �� ����

	BuildFrameBuffer();                   // ������ ���� ����
	BuildObjects();                       // ���� ������Ʈ ����

	// ������ ����Ʈ ���ڿ� �ʱ�ȭ
	_tcscpy_s(m_pszFrameRate, _T("3DGPProject("));

	return true;
}

void CGameFramework::OnDestroy()
{
	// GPU�� ��� ��� ����Ʈ�� ������ ������ ���
	WaitForGpuComplete();

	// ���� ������Ʈ ����
	ReleaseObjects();

	// �潺 �̺�Ʈ �ڵ� �ݱ�
	::CloseHandle(m_hFenceEvent);

	////////////////////////////////////////////////////////
	// ���� Ÿ�� �� ���� ���ٽ� ���ҽ� ����
	for (int i = 0; i < m_nSwapChainBuffers; ++i)
	{
		if (m_ppd3dRenderTargetBuffers[i])
			m_ppd3dRenderTargetBuffers[i]->Release();
	}

	if (m_pd3dRtvDescriptorHeap)		m_pd3dRtvDescriptorHeap->Release();
	if (m_pd3dDepthStencilBuffer)		m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap)		m_pd3dDsvDescriptorHeap->Release();

	////////////////////////////////////////////////////////
	// Ŀ�ǵ� ���� ��ü ����
	if (m_pd3dCommandAllocator)		m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue)			m_pd3dCommandQueue->Release();
	if (m_pd3dPipelineState)		m_pd3dPipelineState->Release();
	if (m_pd3dCommandList)			m_pd3dCommandList->Release();

	////////////////////////////////////////////////////////
	// ����ȭ ��ü ����
	if (m_pd3dFence)				m_pd3dFence->Release();

	////////////////////////////////////////////////////////
	// ���� ü�� �� ����̽� ����
	if (m_pdxgiSwapChain)			m_pdxgiSwapChain->Release();
	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);
	if (m_pd3dDevice)				m_pd3dDevice->Release();
	if (m_pdxgiFactory)				m_pdxgiFactory->Release();

	////////////////////////////////////////////////////////
	// ����׿� ���̺� ��ü ����Ʈ (_DEBUG ��ũ�� Ȱ�� ��)
#if defined(_DEBUG)
	{
		IDXGIDebug1* pdxgiDebug = nullptr;
		DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
		HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
			DXGI_DEBUG_RLO_DETAIL);
		pdxgiDebug->Release();
	}
#endif

}

void CGameFramework::CreateDirect3DDevice()
{
	HRESULT hResult;
	UINT nDXGIFactoryFlags = 0;

	////////////////////////////////////////////////////////
	// ����� ���� Ȱ��ȭ (����� ������ ����)
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void
		**)&pd3dDebugController);
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif
	////////////////////////////////////////////////////////
	// DXGI ���丮 ����
	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void
		**)&m_pdxgiFactory);

	////////////////////////////////////////////////////////
	// �ϵ���� ����� ���� �� ����̽� ����
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);

		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;

		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0,
			_uuidof(ID3D12Device), (void**)&m_pd3dDevice))) 
			break; // ���������� ����̽� ���� �� ���� ����
	}

	// �ϵ���� ����̽� ���� ���� �� WARP ����̽��� ��ü
	if (!m_pd3dDevice)
	{
		m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0,
			__uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}

	////////////////////////////////////////////////////////
	// MSAA 4x ǰ�� ���� Ȯ��
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels = {};
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;

	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));

	m_nMsaa4xQualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels > 1) ? true : false;

	////////////////////////////////////////////////////////
	// Fence ���� �� �̺�Ʈ ��ü ����
	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence),
		(void**)&m_pd3dFence);

	m_nFenceValue = 0;

	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);

	////////////////////////////////////////////////////////
	// ����Ʈ �� ���� �簢�� ����
	m_d3dViewport.TopLeftX = 0;
	m_d3dViewport.TopLeftY = 0;
	m_d3dViewport.Width = static_cast<float>(m_nWndClientWidth);
	m_d3dViewport.Height = static_cast<float>(m_nWndClientHeight);
	m_d3dViewport.MinDepth = 0.0f;
	m_d3dViewport.MaxDepth = 1.0f;

	m_d3dScissorRect = { 0, 0, m_nWndClientWidth, m_nWndClientHeight };
	
	////////////////////////////////////////////////////////
	// ����� ����
	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFramework::CreateCommandQueueAndList()
{
	////////////////////////////////////////////////////////
	// Ŀ�ǵ� ť ����
	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc = {};
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;	// ���� ���� Ÿ��
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(
		&d3dCommandQueueDesc,
		__uuidof(ID3D12CommandQueue),
		reinterpret_cast<void**>(&m_pd3dCommandQueue));

	////////////////////////////////////////////////////////
	// Ŀ�ǵ� �Ҵ��� ����
	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		__uuidof(ID3D12CommandAllocator), (void**)&m_pd3dCommandAllocator);
	
	////////////////////////////////////////////////////////
	// Ŀ�ǵ� ����Ʈ ����
	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void
			**)&m_pd3dCommandList);
	
	// Ŀ�ǵ� ����Ʈ�� ���� �� ���� �����̹Ƿ� �ݾ��ش�
	hResult = m_pd3dCommandList->Close();
}

void CGameFramework::CreateSwapChain()
{
	////////////////////////////////////////////////////////
	// Ŭ���̾�Ʈ ���� ũ�� ����
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;


	////////////////////////////////////////////////////////
	// ���� ü�� ��ũ���� ���� (DXGI 1.2 �̻�)
	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc = {};
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = m_bMsaa4xEnable ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = m_bMsaa4xEnable ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = 0;

	////////////////////////////////////////////////////////
	// ��üȭ�� ��ũ���� ����
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc = {};
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;

	////////////////////////////////////////////////////////
	// ���� ü�� ����
	m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd,
		&dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1
			**)&m_pdxgiSwapChain);

	////////////////////////////////////////////////////////
	// Alt+Enter ��Ȱ��ȭ (��üȭ�� ��ȯ ����)
	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	////////////////////////////////////////////////////////
	// ���� ����� �ε��� ȹ��
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();
}

void CGameFramework::CreateRtvAndDsvDescriptorHeaps()
{
	////////////////////////////////////////////////////////
	// RTV (Render Target View) ������ �� ����
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;

	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);

	m_nRtvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	////////////////////////////////////////////////////////
	// DSV (Depth Stencil View) ������ �� ����
	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc,
		__uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);

	m_nDsvDescriptorIncrementSize =
		m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateRenderTargetViews()
{
	////////////////////////////////////////////////////////
	// ���� Ÿ�� ��(RTV) ���� �� ����ü�� ���� ����
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	for (UINT i = 0; i < m_nSwapChainBuffers; i++)
	{
		m_pdxgiSwapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize; // ���� RTV ��ġ�� ������ ����
	}
}

void CGameFramework::CreateDepthStencilView()
{
	////////////////////////////////////////////////////////
	// ����-���ٽ� ���ҽ� ���� ����ü ����
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	////////////////////////////////////////////////////////
	// �� �Ӽ� ���� (�⺻ GPU ���� ���� ��)
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	////////////////////////////////////////////////////////
	// ����-���ٽ� �ʱ�ȭ Ŭ���� �� ����
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	////////////////////////////////////////////////////////
	// ����-���ٽ� ���� ���ҽ� ����
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE,
		&d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue,
		__uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);

	////////////////////////////////////////////////////////
	// ����-���ٽ� �� ����
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL,
		d3dDsvCPUDescriptorHandle);

}

//void CGameFramework::BuildFrameBuffer()
//{
//	::GetClientRect(m_hWnd, &m_rcClient);
//
//	HDC hDC = ::GetDC(m_hWnd);
//
//	m_hDCFrameBuffer = ::CreateCompatibleDC(hDC);
//	m_hBitmapFrameBuffer = ::CreateCompatibleBitmap(hDC, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top);
//	::SelectObject(m_hDCFrameBuffer, m_hBitmapFrameBuffer);
//
//	::ReleaseDC(m_hWnd, hDC);
//	::SetBkMode(m_hDCFrameBuffer, TRANSPARENT);
//}
//
//void CGameFramework::ClearFrameBuffer(DWORD dwColor)
//{
//	HPEN hPen = ::CreatePen(PS_SOLID, 0, dwColor);
//	HPEN hOldPen = (HPEN)::SelectObject(m_hDCFrameBuffer, hPen);
//	HBRUSH hBrush = ::CreateSolidBrush(dwColor);
//	HBRUSH hOldBrush = (HBRUSH)::SelectObject(m_hDCFrameBuffer, hBrush);
//	::Rectangle(m_hDCFrameBuffer, m_rcClient.left, m_rcClient.top, m_rcClient.right, m_rcClient.bottom);
//	::SelectObject(m_hDCFrameBuffer, hOldBrush);
//	::SelectObject(m_hDCFrameBuffer, hOldPen);
//	::DeleteObject(hPen);
//	::DeleteObject(hBrush);
//}
//
//void CGameFramework::PresentFrameBuffer()
//{    
//    HDC hDC = ::GetDC(m_hWnd);
//    ::BitBlt(hDC, m_rcClient.left, m_rcClient.top, m_rcClient.right - m_rcClient.left, m_rcClient.bottom - m_rcClient.top, m_hDCFrameBuffer, m_rcClient.left, m_rcClient.top, SRCCOPY);
//    ::ReleaseDC(m_hWnd, hDC);
//}

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

void CGameFramework::WaitForGpuComplete()
{
	// ���� ���� ü�� ���ۿ� ���� �潺 �� ���� �� ���
	UINT64 nFenceValue = ++m_nFenceValue[m_nSwapChainBufferIndex];

	// Ŀ�ǵ� ť�� �潺 ��ȣ ����
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	// GPU�� ���� �ش� �潺 ���� �������� ���ߴٸ�, �̺�Ʈ�� �����ϰ� ���
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::MoveToNextFrame()
{
	//////////////////////////////////////////
	// ���� ���� ü�� ���� �ε��� ����
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	//////////////////////////////////////////
	// GPU ����ȭ: �̹� ������ �潺 �� ���� �� ���
	UINT64 nFenceValue = ++m_nFenceValue[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}

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
	////////////////////////////////////////////////////////
	// ������ Ÿ�̸� �� �Է�, �ִϸ��̼� ó��
	m_GameTimer.Tick(60.0f);
	ProcessInput();
	AnimateObjects();

	////////////////////////////////////////////////////////
	// Ŀ�ǵ� ����Ʈ �ʱ�ȭ
	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	m_pd3dCommandList->RSSetViewports(1, &m_d3dViewport);
	m_pd3dCommandList->RSSetScissorRects(1, &m_d3dScissorRect);

	////////////////////////////////////////////////////////
	// ���� Ÿ�� ���� ��ȯ (Present �� Render Target)
	D3D12_RESOURCE_BARRIER d3dResourceBarrier = {};
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	////////////////////////////////////////////////////////
	// ���� Ÿ�� �� ���� ���ٽ� ����
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle =
		m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle =
		m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, FALSE, &d3dDsvCPUDescriptorHandle);

	////////////////////////////////////////////////////////
	// ���� Ÿ�� �� ���� ���� �ʱ�ȭ
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor, 0, nullptr);
	m_pd3dCommandList->ClearDepthStencilView(
		d3dDsvCPUDescriptorHandle,
		D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL,
		1.0f, 0, 0, nullptr
	);

	////////////////////////////////////////////////////////
	// ���� �������� ������
	CCamera* pCamera = m_pPlayer[manager->getCurrLevel()]->GetCamera();
	if (manager->getCurrStage())
		manager->getCurrStage()->Render(m_hDCFrameBuffer, pCamera);

	////////////////////////////////////////////////////////
	// ���� Ÿ�� ���� ��ȯ (Render Target �� Present)
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	////////////////////////////////////////////////////////
	// Ŀ�ǵ� ����Ʈ ���� �� GPU ����ȭ
	hResult = m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(_countof(ppd3dCommandLists), ppd3dCommandLists);
	WaitForGpuComplete();

	////////////////////////////////////////////////////////
	// ������ ��� �� ���� ������ �غ�
	m_pdxgiSwapChain->Present(0, 0);
	MoveToNextFrame();

	////////////////////////////////////////////////////////
	// ������ �ӵ� ���� �� ������ Ÿ��Ʋ ǥ��
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);

}