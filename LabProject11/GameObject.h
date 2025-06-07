#pragma once
#include "Mesh.h"
#include "Camera.h"
class CShader;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();
private:
	int m_nReferences = 0;
public:
	bool m_bActive = true;
	bool m_bBlowingUp = false; // 폭발 여부

	XMFLOAT3					m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float						m_fMovingSpeed = 0.0f;
	float						m_fMovingRange = 0.0f;

	XMFLOAT3					m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	float						m_fRotationSpeed = 0.0f;

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
public:
	XMFLOAT4X4 m_xmf4x4World;
	CMesh* m_pMesh = NULL;
	CShader* m_pShader = NULL;
public:
	void ReleaseUploadBuffers();
	virtual void SetMesh(CMesh* pMesh);
	virtual void SetShader(CShader* pShader);
	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh);
public:
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);
	void Move(XMFLOAT3& vDirection, float fSpeed);
	//따라하기11
public:
	//상수 버퍼를 생성한다.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	//상수 버퍼의 내용을 갱신한다.
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향(x-축, y-축, z-축) 벡터를 반환한다.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	//게임 객체의 위치를 설정한다.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetRotationTransform(XMFLOAT4X4* pmxf4x4Transform);
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }

	//게임 객체를 로컬 x-축, y-축, z-축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	//게임 객체를 회전(x-축, y-축, z-축)한다.
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View,
		XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection);
	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance);

	void UpdateBoundingBox();

	void SetActive(bool bActive) { m_bActive = bActive; }
	int getTargetStage();
};

class CRotatingObject : public CGameObject
{
public:
	CRotatingObject();
	virtual ~CRotatingObject();
private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed;
public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) {
		m_xmf3RotationAxis =
			xmf3RotationAxis;
	}
	virtual void Animate(float fTimeElapsed);
};

class CExplosiveObject : public CGameObject
{
public:
	CExplosiveObject(); // 생성자
	virtual ~CExplosiveObject(); // 소멸자

	// ─────────────── 폭발 관련 상태 ───────────────
	XMFLOAT4X4 m_pxmf4x4Transforms[EXPLOSION_DEBRISES]; // 각 파편의 월드 변환 행렬

	float m_fElapsedTimes = 0.0f;     // 폭발 시작 이후 경과 시간
	float m_fDuration = 2.0f;         // 폭발 지속 시간 (초 단위)
	float m_fExplosionSpeed = 10.0f;  // 파편 확산 속도
	float m_fExplosionRotation = 720.0f; // 파편 회전 속도 (도 단위, 예: 720도는 두 바퀴)

	bool m_bDestroyed = false; // 오브젝트 파괴 완료 여부

	static CMesh* m_pExplosionMesh; // 폭발 시 사용되는 정적 메시 자원 (공용)

	static XMFLOAT3 m_pxmf3SphereVectors[EXPLOSION_DEBRISES]; // 파편의 퍼지는 방향 벡터들

	// ─────────────── 총알 관련 상태 ───────────────
	bool m_bIsBullet = false;     // 총알인지 여부 (true면 총알 로직 적용)
	float m_fBulletLife = 0.0f;   // 현재 총알 생존 시간
	float m_fBulletMaxLife = 200.0f; // 총알 최대 생존 시간

	// ─────────────── 정적 초기화 함수 ───────────────
	static void PrepareExplosion(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList); // 파편 방향 벡터 및 메시 초기화

private:
	// ─────────────── 회전 관련 데이터 ───────────────
	XMFLOAT3 m_xmf3RotationAxis;  // 오브젝트의 회전 축
	float m_fRotationSpeed;       // 회전 속도

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; } // 회전 속도 설정
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; } // 회전 축 설정

	virtual void Animate(float fTimeElapsed); // 시간 기반 애니메이션 (폭발 진행 처리)
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	int getTargetStage(); // 오브젝트가 속한 타겟 스테이지 반환

};

class CBulletObject : public CGameObject
{
public:
	CBulletObject(float fEffectiveRange);
	virtual ~CBulletObject();

public:
	virtual void Animate(float fElapsedTime);

	float						m_fTimeSinceLastFire = 0.0f;
	float						m_fFireCooldown = 0.0f;

	float						m_fBulletEffectiveRange = 50.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 4.0f;
	CGameObject* m_pLockedObject = NULL;

	void SetFirePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();
};
