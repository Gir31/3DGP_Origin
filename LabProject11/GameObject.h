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
	bool m_bBlowingUp = false; // ���� ����

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
	//�����ϱ�11
public:
	//��� ���۸� �����Ѵ�.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList);
	//��� ������ ������ �����Ѵ�.
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	//���� ��ü�� ���� ��ȯ ��Ŀ��� ��ġ ���Ϳ� ����(x-��, y-��, z-��) ���͸� ��ȯ�Ѵ�.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	//���� ��ü�� ��ġ�� �����Ѵ�.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetRotationTransform(XMFLOAT4X4* pmxf4x4Transform);
	void SetMovingDirection(XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }

	//���� ��ü�� ���� x-��, y-��, z-�� �������� �̵��Ѵ�.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	//���� ��ü�� ȸ��(x-��, y-��, z-��)�Ѵ�.
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
	CExplosiveObject(); // ������
	virtual ~CExplosiveObject(); // �Ҹ���

	// ������������������������������ ���� ���� ���� ������������������������������
	XMFLOAT4X4 m_pxmf4x4Transforms[EXPLOSION_DEBRISES]; // �� ������ ���� ��ȯ ���

	float m_fElapsedTimes = 0.0f;     // ���� ���� ���� ��� �ð�
	float m_fDuration = 2.0f;         // ���� ���� �ð� (�� ����)
	float m_fExplosionSpeed = 10.0f;  // ���� Ȯ�� �ӵ�
	float m_fExplosionRotation = 720.0f; // ���� ȸ�� �ӵ� (�� ����, ��: 720���� �� ����)

	bool m_bDestroyed = false; // ������Ʈ �ı� �Ϸ� ����

	static CMesh* m_pExplosionMesh; // ���� �� ���Ǵ� ���� �޽� �ڿ� (����)

	static XMFLOAT3 m_pxmf3SphereVectors[EXPLOSION_DEBRISES]; // ������ ������ ���� ���͵�

	// ������������������������������ �Ѿ� ���� ���� ������������������������������
	bool m_bIsBullet = false;     // �Ѿ����� ���� (true�� �Ѿ� ���� ����)
	float m_fBulletLife = 0.0f;   // ���� �Ѿ� ���� �ð�
	float m_fBulletMaxLife = 200.0f; // �Ѿ� �ִ� ���� �ð�

	// ������������������������������ ���� �ʱ�ȭ �Լ� ������������������������������
	static void PrepareExplosion(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList); // ���� ���� ���� �� �޽� �ʱ�ȭ

private:
	// ������������������������������ ȸ�� ���� ������ ������������������������������
	XMFLOAT3 m_xmf3RotationAxis;  // ������Ʈ�� ȸ�� ��
	float m_fRotationSpeed;       // ȸ�� �ӵ�

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; } // ȸ�� �ӵ� ����
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; } // ȸ�� �� ����

	virtual void Animate(float fTimeElapsed); // �ð� ��� �ִϸ��̼� (���� ���� ó��)
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	int getTargetStage(); // ������Ʈ�� ���� Ÿ�� �������� ��ȯ

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
