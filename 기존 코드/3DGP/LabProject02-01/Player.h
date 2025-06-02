#pragma once

#include "GameObject.h"
#include "Camera.h"

class CPlayer : public CGameObject
{
public:
	CPlayer();
	virtual ~CPlayer();

public:
	XMFLOAT3					m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	XMFLOAT3					m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	XMFLOAT3					m_xmf3CameraOffset = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3					m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	float						m_fFriction = 125.0f;

	float           			m_fPitch = 30.0f; 
	float           			m_fYaw = 0.0f;
	float           			m_fRoll = 0.0f;

	CCamera*					m_pCamera = NULL;

	float m_fCameraYaw = 0.0f;     
	float m_fCameraPitch = 30.0f; 

	float m_fCameraDistance = 40.0f; 
	bool m_bOrbitMode = false; 

	float m_fSavedYaw = 0.0f;
	float m_fSavedPitch = 30.0f;

	CPlayer* m_pParent = nullptr; 
	bool m_bIsLButtonDown = false;

	//============================

public:
	void SetPosition(float x, float y, float z);
	void SetRotation(float x, float y, float z);

	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	virtual void Move(DWORD dwDirection, float fDistance);
	void Move(XMFLOAT3& xmf3Shift, bool bUpdateVelocity);
	void Move(float x, float y, float z);

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);

	void SetCameraOffset(XMFLOAT3& xmf3CameraOffset);

	void Update(float fTimeElapsed = 0.016f);

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);

	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }
	CCamera* GetCamera() { return(m_pCamera); }
	void SetParent(CPlayer* pParent) { m_pParent = pParent; }
};

#define BULLETS					50

class CCursor : public CPlayer
{
public:
	CCursor();
	virtual ~CCursor();

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);
};

class CCart : public CPlayer
{
private:
	float m_fPathTime = 0.0f; 
	int m_iPathIndex = 0;
public:
	CCart();
	virtual ~CCart();

	bool eFlag = false;

	XMFLOAT3 CatmullRom(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT3& p3, float t);

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);
	
	bool getFlag();
};

class CTankTurret : public CPlayer
{
private:
	float m_fTurretYaw = 0.0f;       
	float m_fPrevCameraYaw = 0.0f; 
public:
	CTankTurret(CCamera* pCamera, CMesh* pTurretMesh);
	virtual ~CTankTurret() {}

	CBulletObject* m_ppBullets[BULLETS]{};
	float m_fBulletEffectiveRange = 200.0f;

	void FireBullet(CGameObject* pLockedObject);

	virtual void OnUpdateTransform();
	virtual void Animate(float fElapsedTime);

	void AddYaw(float fDeltaYaw);
};

class CTank : public CPlayer
{
public:
	CTank(CCamera* pCamera, CMesh* pTurretMesh);
	virtual ~CTank();

	CTankTurret* p_pTurret = nullptr;

	virtual void OnUpdateTransform() override;
	virtual void Animate(float fElapsedTime) override;

	virtual void Move(DWORD dwDirection, float fDistance) override;
	virtual void Rotate(float fPitch, float fYaw, float fRoll) override;

	void FireBullet(CGameObject* pLockedObject);
private:
	bool m_bIsMoving = false; 
};