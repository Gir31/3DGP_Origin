#include "stdafx.h"
#include "Player.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPlayer::CPlayer()
{
}

CPlayer::~CPlayer()
{
	if (m_pCamera) delete m_pCamera;
}

void CPlayer::SetPosition(float x, float y, float z)
{
	m_xmf3Position = XMFLOAT3(x, y, z);

	CGameObject::SetPosition(x, y, z);
}

void CPlayer::SetCameraOffset(XMFLOAT3& xmf3CameraOffset)
{
	m_xmf3CameraOffset = xmf3CameraOffset;
	m_pCamera->SetLookAt(Vector3::Add(m_xmf3Position, m_xmf3CameraOffset), m_xmf3Position, m_xmf3Up);
	m_pCamera->GenerateViewMatrix();
}

void CPlayer::Move(DWORD dwDirection, float fDistance)
{
	if (dwDirection && m_bOrbitMode)
	{
		float yaw = XMConvertToRadians(m_fCameraYaw);

		m_xmf3Look.x = -sinf(yaw);
		m_xmf3Look.y = 0.0f;
		m_xmf3Look.z = cosf(yaw);
		m_xmf3Look = Vector3::Normalize(m_xmf3Look);

		m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(XMFLOAT3(0.0f, 1.0f, 0.0f), m_xmf3Look));
		m_xmf3Up = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));
	}

	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);
		if (dwDirection & DIR_FORWARD)  xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		if (dwDirection & DIR_RIGHT)    xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT)     xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		if (dwDirection & DIR_UP)       xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN)     xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		Move(xmf3Shift, true);
	}
}

void CPlayer::Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	if (bUpdateVelocity)
	{
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		m_xmf3Position = Vector3::Add(xmf3Shift, m_xmf3Position);
		m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Move(float x, float y, float z)
{
	Move(XMFLOAT3(x, y, z), false);
}

void CPlayer::Rotate(float fPitch, float fYaw, float fRoll)
{
	m_pCamera->Rotate(fPitch, fYaw, fRoll);
	if (fPitch != 0.0f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(fPitch));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRotate);
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, mtxRotate);
	}
	if (fYaw != 0.0f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(fYaw));
		m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, mtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRotate);
	}
	if (fRoll != 0.0f)
	{
		XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(fRoll));
		m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, mtxRotate);
		m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, mtxRotate);
	}

	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Up, m_xmf3Look));
	m_xmf3Up = Vector3::Normalize(Vector3::CrossProduct(m_xmf3Look, m_xmf3Right));
}

void CPlayer::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(m_xmf3Position, xmf3LookAt, xmf3Up);
	m_xmf3Right = Vector3::Normalize(XMFLOAT3(xmf4x4View._11, xmf4x4View._21, xmf4x4View._31));
	m_xmf3Up = Vector3::Normalize(XMFLOAT3(xmf4x4View._12, xmf4x4View._22, xmf4x4View._32));
	m_xmf3Look = Vector3::Normalize(XMFLOAT3(xmf4x4View._13, xmf4x4View._23, xmf4x4View._33));
}

void CPlayer::Update(float fTimeElapsed)
{
	Move(m_xmf3Velocity, false);

	m_pCamera->Update(this, m_xmf3Position, fTimeElapsed);
	m_pCamera->GenerateViewMatrix();

	XMFLOAT3 xmf3Deceleration = Vector3::Normalize(Vector3::ScalarProduct(m_xmf3Velocity, -1.0f));
	float fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = m_fFriction * fTimeElapsed;
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Deceleration, fDeceleration);
}

void CPlayer::Animate(float fElapsedTime)
{
	OnUpdateTransform();

	CGameObject::Animate(fElapsedTime);
}

void CPlayer::OnUpdateTransform()
{
	m_xmf4x4World._11 = m_xmf3Right.x; m_xmf4x4World._12 = m_xmf3Right.y; m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x; m_xmf4x4World._22 = m_xmf3Up.y; m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x; m_xmf4x4World._32 = m_xmf3Look.y; m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z;
}

/////////////////////////////////////////////////////////////////////////////////////////////
//
CCursor::CCursor()
{
	CCubeMesh* pBulletMesh = new CCubeMesh(1.0f, 4.0f, 1.0f);
}

CCursor::~CCursor()
{
}

void CCursor::Animate(float fElapsedTime)
{
	CPlayer::Animate(fElapsedTime);
}

void CCursor::OnUpdateTransform()
{
	CPlayer::OnUpdateTransform();

	m_xmf4x4World = Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f), m_xmf4x4World);
}
/////////////////////////////////////////////////////////////////////////////////////////////
//
CCart::CCart() 
{
}

CCart::~CCart() 
{
}

std::array<XMFLOAT3, 41> curvePoints = {
	XMFLOAT3{0.0f, 0.0f, 0.0f},
	XMFLOAT3{10.0f, 50.0f, 30.0f},
	XMFLOAT3{20.0f, -50.0f, 60.0f},
	XMFLOAT3{10.0f, 70.0f, 90.0f},
	XMFLOAT3{-10.0f, -30.0f, 120.0f},
	XMFLOAT3{-30.0f, 50.0f, 150.0f},
	XMFLOAT3{-10.0f, -10.0f, 180.0f},
	XMFLOAT3{0.0f, 30.0f, 210.0f},
	XMFLOAT3{20.0f, 50.0f, 240.0f},
	XMFLOAT3{40.0f, 50.0f, 270.0f},
	XMFLOAT3{50.0f, -50.0f, 300.0f},
	XMFLOAT3{55.0f, -20.0f, 330.0f},
	XMFLOAT3{40.0f, -30.0f, 360.0f},
	XMFLOAT3{30.0f, 10.0f, 390.0f},
	XMFLOAT3{20.0f, -50.0f, 420.0f},
	XMFLOAT3{10.0f, -20.0f, 450.0f},
	XMFLOAT3{0.0f, -40.0f, 480.0f},
	XMFLOAT3{10.0f, -70.0f, 510.0f},
	XMFLOAT3{0.0f, -100.0f, 540.0f},
	XMFLOAT3{-20.0f, -50.0f, 570.0f},
	XMFLOAT3{-20.0f, -30.0f, 600.0f},
	XMFLOAT3{-10.0f, 0.0f, 630.0f},
	XMFLOAT3{10.0f, 50.0f, 660.0f},
	XMFLOAT3{20.0f, 70.0f, 690.0f},
	XMFLOAT3{30.0f, 80.0f, 720.0f},
	XMFLOAT3{20.0f, 50.0f, 750.0f},
	XMFLOAT3{30.0f, 30.0f, 780.0f},
	XMFLOAT3{20.0f, 50.0f, 810.0f},
	XMFLOAT3{-10.0f, 20.0f, 840.0f},
	XMFLOAT3{0.0f, 0.0f, 870.0f},
	XMFLOAT3{10.0f, -50.0f, 900.0f},
	XMFLOAT3{30.0f, 50.0f, 930.0f},
	XMFLOAT3{20.0f, -10.0f, 960.0f},
	XMFLOAT3{40.0f, 20.0f, 990.0f},
	XMFLOAT3{50.0f, 50.0f, 1020.0f},
	XMFLOAT3{60.0f, 70.0f, 1050.0f},
	XMFLOAT3{70.0f, 80.0f, 1080.0f},
	XMFLOAT3{50.0f, 50.0f, 1110.0f},
	XMFLOAT3{40.0f, 0.0f, 1140.0f},
	XMFLOAT3{30.0f, 0.0f, 1170.0f},
	XMFLOAT3{20.0f, 0.0f, 1200.0f},
};

void CCart::Animate(float fElapsedTime)
{
	m_fPathTime += fElapsedTime;
	if (m_fPathTime >= 1.0f)
	{
		m_fPathTime = 0.0f;
		m_iPathIndex++;
	}

	if (m_iPathIndex <= curvePoints.size() - 4) {
		int count = static_cast<int>(curvePoints.size());
		if (count < 4) return;

		int i0 = m_iPathIndex % count;
		int i1 = (m_iPathIndex + 1) % count;
		int i2 = (m_iPathIndex + 2) % count;
		int i3 = (m_iPathIndex + 3) % count;

		XMFLOAT3 currPos = CatmullRom(curvePoints[i0], curvePoints[i1], curvePoints[i2], curvePoints[i3], m_fPathTime);
		float nextT = min(m_fPathTime + 0.01f, 1.0f);
		XMFLOAT3 nextPos = CatmullRom(curvePoints[i0], curvePoints[i1], curvePoints[i2], curvePoints[i3], nextT);

		XMFLOAT3 forward = Vector3::Normalize(Vector3::Subtract(nextPos, currPos));
		XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(XMFLOAT3(0, 1, 0), forward)); 
		XMFLOAT3 up = Vector3::CrossProduct(forward, right); 

		m_xmf3Look = forward;
		m_xmf3Right = right;
		m_xmf3Up = up;

		SetPosition(currPos.x, currPos.y, currPos.z);
		OnUpdateTransform(); 

		SetPosition(currPos.x, currPos.y, currPos.z);
	}
	else {
		eFlag = true;
	}

	CPlayer::Animate(fElapsedTime);
}

void CCart::OnUpdateTransform()
{
	CPlayer::OnUpdateTransform();

	m_xmf4x4World = Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f), m_xmf4x4World);
}

bool CCart::getFlag()
{
	return eFlag;
}

XMFLOAT3 CCart::CatmullRom(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT3& p3, float t)
{
	XMVECTOR v0 = XMLoadFloat3(&p0);
	XMVECTOR v1 = XMLoadFloat3(&p1);
	XMVECTOR v2 = XMLoadFloat3(&p2);
	XMVECTOR v3 = XMLoadFloat3(&p3);

	float t2 = t * t;
	float t3 = t2 * t;

	XMVECTOR result =
		0.5f * (
			(2.0f * v1) +
			(-v0 + v2) * t +
			(2.0f * v0 - 5.0f * v1 + 4.0f * v2 - v3) * t2 +
			(-v0 + 3.0f * v1 - 3.0f * v2 + v3) * t3
			);

	XMFLOAT3 final;
	XMStoreFloat3(&final, result);
	return final;
}
/////////////////////////////////////////////////////////////////////////////////////////////
//
CTank::CTank(CCamera* pCamera, CMesh* pTurretMesh)
{
	p_pTurret = new CTankTurret(pCamera, pTurretMesh);
	p_pTurret->SetParent(this);
}

CTank::~CTank()
{
}

void CTank::Animate(float fElapsedTime)
{
	float fDeltaYaw = m_fCameraYaw - m_fSavedYaw;

	p_pTurret->AddYaw(fDeltaYaw); 

	m_fSavedYaw = m_fCameraYaw;


	p_pTurret->Animate(fElapsedTime);
	CPlayer::Animate(fElapsedTime);
}

void CTank::OnUpdateTransform()
{
	p_pTurret->OnUpdateTransform();

	CPlayer::OnUpdateTransform();

	m_xmf4x4World = Matrix4x4::Multiply(XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f), m_xmf4x4World);
}

void CTank::Rotate(float fPitch, float fYaw, float fRoll)
{
	CPlayer::Rotate(fPitch, fYaw, fRoll);
}

void CTank::Move(DWORD dwDirection, float fDistance)
{
	m_bIsMoving = (dwDirection != 0);

	XMFLOAT3 desiredDirection = { 0, 0, 0 };

	if (m_bIsMoving)
	{
		// 카메라 기준 벡터
		XMFLOAT3 cameraLook = Vector3::Normalize(XMFLOAT3(-sinf(XMConvertToRadians(m_fCameraYaw)), 0.0f, cosf(XMConvertToRadians(m_fCameraYaw))));
		XMFLOAT3 cameraRight = Vector3::Normalize(Vector3::CrossProduct(XMFLOAT3(0, 1, 0), cameraLook));

		// 원하는 이동 방향 계산
		if (dwDirection & DIR_FORWARD)  desiredDirection = Vector3::Add(desiredDirection, cameraLook);
		if (dwDirection & DIR_BACKWARD) desiredDirection = Vector3::Add(desiredDirection, Vector3::ScalarProduct(cameraLook, -1.0f));
		if (dwDirection & DIR_RIGHT)    desiredDirection = Vector3::Add(desiredDirection, cameraRight);
		if (dwDirection & DIR_LEFT)     desiredDirection = Vector3::Add(desiredDirection, Vector3::ScalarProduct(cameraRight, -1.0f));

		desiredDirection = Vector3::Normalize(desiredDirection);

		// 현재 정면과 회전각 계산
		XMFLOAT3 lookDir = Vector3::Normalize(m_xmf3Look);
		float dot = Vector3::DotProduct(lookDir, desiredDirection);
		float angle = acosf(std::clamp(dot, -1.0f, 1.0f));
		XMFLOAT3 cross = Vector3::CrossProduct(lookDir, desiredDirection);
		if (cross.y < 0) angle = -angle;

		float targetDegree = XMConvertToDegrees(angle);

		// 부드럽게 회전: 최대 회전 속도 제한
		float maxTurnSpeedPerFrame = 30.0f * fDistance; // 회전 속도 = 회전율 * 이동 거리 기반
		float clampedDegree = std::clamp(targetDegree, -maxTurnSpeedPerFrame, maxTurnSpeedPerFrame);

		this->Rotate(0.0f, clampedDegree, 0.0f);

		// 방향 갱신 이후 실제 이동은 "현재 m_xmf3Look 기준"으로만
		desiredDirection = Vector3::ScalarProduct(m_xmf3Look, fDistance  * 5);
		CPlayer::Move(desiredDirection, true); // CPlayer::Move(xmf3Shift, true)
	}

	m_bIsMoving = false;
}

void CTank::FireBullet(CGameObject* pLockedObject)
{ 
	OutputDebugStringA("[CTank] FireBullet 호출됨\n");
	p_pTurret->FireBullet(pLockedObject);
}

CTankTurret::CTankTurret(CCamera* pCamera, CMesh* pTurretMesh)
{
	SetPosition(0.f, 0.f, 0.f);
	SetMesh(pTurretMesh);
	SetColor(RGB(0, 0, 255));
	SetCamera(pCamera);
	SetCameraOffset(XMFLOAT3(0.0f, 5.0f, -15.0f));

	CCubeMesh* pBulletMesh = new CCubeMesh(0.5f, 2.0f, 0.5f);
	for (int i = 0; i < BULLETS; ++i)
	{
		m_ppBullets[i] = new CBulletObject(m_fBulletEffectiveRange);
		m_ppBullets[i]->SetMesh(pBulletMesh);
		m_ppBullets[i]->SetRotationAxis(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_ppBullets[i]->SetRotationSpeed(360.0f);
		m_ppBullets[i]->SetMovingSpeed(100.0f);
		m_ppBullets[i]->SetActive(false);
	}
}

void CTankTurret::Animate(float fElapsedTime)
{
	CPlayer::Animate(fElapsedTime);

	for (int i = 0; i < BULLETS; ++i) {
		if (m_ppBullets[i]->m_bActive)
			m_ppBullets[i]->Animate(fElapsedTime);
	}
}

void CTankTurret::OnUpdateTransform()
{
	if (m_pParent)
	{
		m_xmf3Position = Vector3::Add(m_pParent->m_xmf3Position, XMFLOAT3(0.0f, 0.0f, 0.0f));
	}

	CPlayer::OnUpdateTransform();

	XMMATRIX xmWorld = XMLoadFloat4x4(&m_xmf4x4World);

	XMMATRIX xmTurretYaw = XMMatrixRotationZ(XMConvertToRadians(m_fTurretYaw));

	XMMATRIX xmYUpToZDown = XMMatrixRotationX(XMConvertToRadians(-90.0f));
	XMMATRIX xmFixUpsideDown = XMMatrixRotationY(XMConvertToRadians(180.0f));
	XMMATRIX xmFixFrontBack = XMMatrixRotationZ(XMConvertToRadians(180.0f));

	xmWorld = xmTurretYaw * xmFixFrontBack * xmFixUpsideDown * xmYUpToZDown * xmWorld;

	XMStoreFloat4x4(&m_xmf4x4World, xmWorld);
}

void CTankTurret::AddYaw(float fDeltaYaw)
{
	m_fTurretYaw += fDeltaYaw;
}

void CTankTurret::FireBullet(CGameObject* pLockedObject)
{
	CBulletObject* pBullet = nullptr;
	for (int i = 0; i < BULLETS; ++i) {
		if (!m_ppBullets[i]->m_bActive) {
			pBullet = m_ppBullets[i];
			break;
		}
	}
	if (pBullet) {

		XMFLOAT3 fireDir = XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23);
		fireDir = Vector3::Normalize(fireDir);

		XMFLOAT3 firePos = Vector3::Add(m_xmf3Position, Vector3::ScalarProduct(fireDir, 35.0f));

		pBullet->m_xmf4x4World = m_xmf4x4World;
		pBullet->SetFirePosition(firePos);
		pBullet->SetMovingDirection(fireDir);
		pBullet->SetColor(RGB(255, 255, 0));
		pBullet->SetActive(true);

		if (pLockedObject)
		{
			pBullet->m_pLockedObject = pLockedObject;
			pBullet->SetColor(RGB(0, 0, 255));
		}

	}

}

