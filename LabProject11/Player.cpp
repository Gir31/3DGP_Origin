#include "Player.h"
#include "Shader.h"

CPlayer::CPlayer()
{
	m_pCamera = NULL;
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;
	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;
	m_pPlayerUpdatedContext = NULL;
	m_pCameraUpdatedContext = NULL;
}
CPlayer::~CPlayer()
{
	ReleaseShaderVariables();
	if (m_pCamera) delete m_pCamera;
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList)
{
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}
void CPlayer::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();
	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}
void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);
}

//�÷��̾��� ��ġ�� �����ϴ� �Լ��̴�. �÷��̾��� ��ġ�� �⺻������ ����ڰ� �÷��̾ �̵��ϱ� ���� Ű���带
//���� �� ����ȴ�. �÷��̾��� �̵� ����(dwDirection)�� ���� �÷��̾ fDistance ��ŭ �̵��Ѵ�.
void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0, 0, 0);

		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look,
			fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);

		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right,
			fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		//��Page Up���� ������ ���� y-�� �������� �̵��Ѵ�. ��Page Down���� ������ �ݴ� �������� �̵��Ѵ�.
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);
		//�÷��̾ ���� ��ġ ���Ϳ��� xmf3Shift ���͸�ŭ �̵��Ѵ�.
		Move(xmf3Shift, bUpdateVelocity);
	}
}
void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bUpdateVelocity)
{
	//bUpdateVelocity�� ���̸� �÷��̾ �̵����� �ʰ� �ӵ� ���͸� �����Ѵ�.
	if (bUpdateVelocity)
	{
		//�÷��̾��� �ӵ� ���͸� xmf3Shift ���͸�ŭ �����Ѵ�.
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		//�÷��̾ ���� ��ġ ���Ϳ��� xmf3Shift ���͸�ŭ �̵��Ѵ�.
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� ī�޶��� ��ġ�� xmf3Shift ���͸�ŭ �̵��Ѵ�.
		if (m_pCamera) m_pCamera->Move(xmf3Shift);
	}
}
//�÷��̾ ���� x-��, y-��, z-���� �߽����� ȸ���Ѵ�.
void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCameraMode = m_pCamera->GetMode();
	//1��Ī ī�޶� �Ǵ� 3��Ī ī�޶��� ��� �÷��̾��� ȸ���� �ణ�� ������ ������.
	if ((nCameraMode == FIRST_PERSON_CAMERA) || (nCameraMode == THIRD_PERSON_CAMERA))
	{
		/*���� x-���� �߽����� ȸ���ϴ� ���� ���� �յڷ� ���̴� ���ۿ� �ش��Ѵ�. �׷��Ƿ� x-���� �߽����� ȸ���ϴ�
	   ������ -89.0~+89.0�� ���̷� �����Ѵ�. x�� ������ m_fPitch���� ���� ȸ���ϴ� �����̹Ƿ� x��ŭ ȸ���� ����
	   Pitch�� +89�� ���� ũ�ų� -89�� ���� ������ m_fPitch�� +89�� �Ǵ� -89���� �ǵ��� ȸ������(x)�� �����Ѵ�.*/
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > +89.0f) { x -= (m_fPitch - 89.0f); m_fPitch = +89.0f; }
			if (m_fPitch < -89.0f) { x -= (m_fPitch + 89.0f); m_fPitch = -89.0f; }
		}
		if (y != 0.0f)
		{
			//���� y-���� �߽����� ȸ���ϴ� ���� ������ ������ ���̹Ƿ� ȸ�� ������ ������ ����.
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			/*���� z-���� �߽����� ȸ���ϴ� ���� ������ �¿�� ����̴� ���̹Ƿ� ȸ�� ������ -20.0~+20.0�� ���̷� ���ѵ�
		   ��. z�� ������ m_fRoll���� ���� ȸ���ϴ� �����̹Ƿ� z��ŭ ȸ���� ���� m_fRoll�� +20�� ���� ũ�ų� -20������
		   ������ m_fRoll�� +20�� �Ǵ� -20���� �ǵ��� ȸ������(z)�� �����Ѵ�.*/
			m_fRoll += z;
			if (m_fRoll > +20.0f) { z -= (m_fRoll - 20.0f); m_fRoll = +20.0f; }
			if (m_fRoll < -20.0f) { z -= (m_fRoll + 20.0f); m_fRoll = -20.0f; }
		}
		//ī�޶� x, y, z ��ŭ ȸ���Ѵ�. �÷��̾ ȸ���ϸ� ī�޶� ȸ���ϰ� �ȴ�.
		m_pCamera->Rotate(x, y, z);
		/*�÷��̾ ȸ���Ѵ�. 1��Ī ī�޶� �Ǵ� 3��Ī ī�޶󿡼� �÷��̾��� ȸ���� ���� y-�࿡���� �Ͼ��. �÷��̾�
	   �� ���� y-��(Up ����)�� �������� ���� z-��(Look ����)�� ���� x-��(Right ����)�� ȸ����Ų��. �⺻������ Up ��
	   �͸� �������� ȸ���ϴ� ���� �÷��̾ �ȹٷ� ���ִ� ���� �����Ѵٴ� �ǹ��̴�.*/
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
				XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCameraMode == SPACESHIP_CAMERA)
	{
		/*�����̽�-�� ī�޶󿡼� �÷��̾��� ȸ���� ȸ�� ������ ������ ����. �׸��� ��� ���� �߽����� ȸ���� �� �� ��
	   ��.*/
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right),
				XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up),
				XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look),
				XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	//ȸ������ ���� �÷��̾��� ���� x-��, y-��, z-���� ���� �������� ���� �� �����Ƿ� z-��(Look ����)�� ��������
   //�Ͽ� ���� �����ϰ� �������Ͱ� �ǵ��� �Ѵ�.
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

//�� �Լ��� �� �����Ӹ��� ȣ��ȴ�. �÷��̾��� �ӵ� ���Ϳ� �߷°� ������ ���� �����Ѵ�.
void CPlayer::Update(float fTimeElapsed)
{
	//�÷��̾��� �ӵ� ���͸� �߷� ���Ϳ� ���Ѵ�. �߷� ���Ϳ� fTimeElapsed�� ���ϴ� ���� �߷��� �ð��� ����ϵ���
   //�����Ѵٴ� �ǹ��̴�.
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity,
		fTimeElapsed, false));

	//�÷��̾��� �ӵ� ������ XZ-������ ũ�⸦ ���Ѵ�. �̰��� XZ-����� �ִ� �ӷº��� ũ�� �ӵ� ������ x�� z-����
	//������ �����Ѵ�
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z *
		m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	//�÷��̾��� �ӵ� ������ y-������ ũ�⸦ ���Ѵ�. �̰��� y-�� ������ �ִ� �ӷº��� ũ�� �ӵ� ������ y-���� ��
   //���� �����Ѵ�.
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);
	//�÷��̾ �ӵ� ���� ��ŭ ������ �̵��Ѵ�(ī�޶� �̵��� ���̴�). 
	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);
	//�÷��̾��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�. �÷��̾��� ���ο� ��ġ�� ��ȿ�� ��ġ�� �ƴ� ����
   //�ְ� �Ǵ� �÷��̾��� �浹 �˻� ���� ������ �ʿ䰡 �ִ�. �̷��� ��Ȳ���� �÷��̾��� ��ġ�� ��ȿ�� ��ġ�� �ٽ�
   //������ �� �ִ�.
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);
	DWORD nCameraMode = m_pCamera->GetMode();
	//�÷��̾��� ��ġ�� ����Ǿ����Ƿ� 3��Ī ī�޶� �����Ѵ�.
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position,
		fTimeElapsed);
	//ī�޶��� ��ġ�� ����� �� �߰��� ������ �۾��� �����Ѵ�. 
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	//ī�޶� 3��Ī ī�޶��̸� ī�޶� ����� �÷��̾� ��ġ�� �ٶ󺸵��� �Ѵ�.
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	//ī�޶��� ī�޶� ��ȯ ����� �ٽ� �����Ѵ�.
	m_pCamera->RegenerateViewMatrix();
	//�÷��̾��� �ӵ� ���Ͱ� ������ ������ ������ �Ǿ�� �Ѵٸ� ���� ���͸� �����Ѵ�. �ӵ� ������ �ݴ� ���� ���͸�
	//���ϰ� ���� ���ͷ� �����. ���� ����� �ð��� ����ϵ��� �Ͽ� �������� ���Ѵ�. ���� ���Ϳ� �������� ���Ͽ� ��
	//�� ���͸� ���Ѵ�. �ӵ� ���Ϳ� ���� ���͸� ���Ͽ� �ӵ� ���͸� ���δ�. �������� �ӷº��� ũ�� �ӷ��� 0�� �� ����
	//��.
	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = (m_fFriction * fTimeElapsed);
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

//ī�޶� ������ �� ChangeCamera() �Լ����� ȣ��Ǵ� �Լ��̴�. nCurrentCameraMode�� ���� ī�޶��� ���
//�̰� nNewCameraMode�� ���� ������ ī�޶� ����̴�.
CCamera* CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	//���ο� ī�޶��� ��忡 ���� ī�޶� ���� �����Ѵ�.
	CCamera* pNewCamera = NULL;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = new CFirstPersonCamera(m_pCamera);
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = new CThirdPersonCamera(m_pCamera);
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = new CSpaceShipCamera(m_pCamera);
		break;
	}

	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;
		/*Look ���Ϳ� ������ǥ���� z-��(0, 0, 1)�� �̷�� ����(����=cos)�� ����Ͽ� �÷��̾��� y-���� ȸ�� ����
	   m_fYaw�� �����Ѵ�.*/
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		/*���ο� ī�޶��� ��尡 �����̽�-�� ����� ī�޶��̰� ���� ī�޶� ��尡 1��Ī �Ǵ� 3��Ī ī�޶��̸� �÷���
	   ���� ���� ���� ���� ī�޶��� ���� ��� ���� �����.*/
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}
	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		//���� ī�޶� ����ϴ� �÷��̾� ��ü�� �����Ѵ�.
		pNewCamera->SetPlayer(this);
	}
	if (m_pCamera) delete m_pCamera;
	return(pNewCamera);
}

//�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�. �÷��̾��� Right ���Ͱ� ���� ��ȯ ���
//�� ù ��° �� ����, Up ���Ͱ� �� ��° �� ����, Look ���Ͱ� �� ��° �� ����, �÷��̾��� ��ġ ���Ͱ� �� ��° ��
//���Ͱ� �ȴ�.
void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;
}
void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//ī�޶� ��尡 3��Ī�̸� �÷��̾� ��ü�� �������Ѵ�.
	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
		CGameObject::Render(pd3dCommandList, pCamera);
	}
}

CAirplanePlayer::CAirplanePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	//����� �޽��� �����Ѵ�.
	CMesh* pCartMesh = new CCartMesh(pd3dDevice, pd3dCommandList);
	SetMesh(pCartMesh);
	//�÷��̾��� ī�޶� �����̽�-�� ī�޶�� ����(����)�Ѵ�.
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	//�÷��̾ ���� ���̴� ������ �����Ѵ�.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	//�÷��̾��� ��ġ�� �����Ѵ�.
	SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));
	//�÷��̾�(�����) �޽��� �������� �� ����� ���̴��� �����Ѵ�.
	CPlayerShader* pShader = new CPlayerShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}
CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
	//����� ���� �׸��� ���� x-������ 90�� ȸ���Ѵ�.
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f,
		0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}
//3��Ī ī�޶��� �� �÷��̾� �޽��� ���� x-���� �߽����� +90�� ȸ���ϰ� �������Ѵ�. �ֳ��ϸ� ����� �� �޽�
//�� ���� �׸��� ���� y-�� ������ ������� ������ �ǵ��� �𵨸��� �Ǿ��� �����̴�. �׸��� �� �޽��� ī�޶��� z
//�� �������� ���ϵ��� �׸� ���̱� �����̴�.



//ī�޶� ������ �� ȣ��Ǵ� �Լ��̴�. nNewCameraMode�� ���� ������ ī�޶� ����̴�. 
CCamera* CAirplanePlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		//�÷��̾��� Ư���� 1��Ī ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�.
		SetFriction(200.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		//�÷��̾��� Ư���� �����̽�-�� ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�.
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(400.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		//�÷��̾��� Ư���� 3��Ī ī�޶� ��忡 �°� �����Ѵ�. ���� ȿ���� ī�޶� �������� �����Ѵ�.
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�. ���� 0.25f ��ſ� 0.0f�� 1.0f�� ������ ����� ���ϱ� �ٶ���.
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	//�÷��̾ �ð��� ����� ���� ����(��ġ�� ������ ����: �ӵ�, ������, �߷� ���� ó��)�Ѵ�.
	Update(fTimeElapsed);
	return(m_pCamera);
}

// Cart
CCartPlayer::CCartPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	CCartMesh* pCartMesh = new CCartMesh(pd3dDevice, pd3dCommandList);
	SetMesh(pCartMesh);
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));
	CPlayerShader* pShader = new CPlayerShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}
CCartPlayer::~CCartPlayer()
{
}

void CCartPlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f,
		0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

XMFLOAT3 CCartPlayer::CatmullRom(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT3& p3, float t)
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

std::array<XMFLOAT3, 41> curvePoints = {
	XMFLOAT3{0.0f, 0.0f, -20.0f},
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

void CCartPlayer::Update(float fElapsedTime)
{
	// īƮ�� ��� ���� �ð� ������Ʈ
	m_fPathTime += fElapsedTime;
	if (m_fPathTime >= 1.0f)
	{
		m_fPathTime = 0.0f;
		m_iPathIndex++;
	}

	// ��ȿ�� � ����Ʈ�� ����� ��쿡�� ����
	if (m_iPathIndex <= curvePoints.size() - 4)
	{
		int count = static_cast<int>(curvePoints.size());
		if (count < 4) return;

		// Catmull-Rom ���� ����Ʈ �ε��� ���
		int i0 = m_iPathIndex % count;
		int i1 = (m_iPathIndex + 1) % count;
		int i2 = (m_iPathIndex + 2) % count;
		int i3 = (m_iPathIndex + 3) % count;

		// ���� ��ġ�� ���� ��ġ ���
		XMFLOAT3 currPos = CatmullRom(curvePoints[i0], curvePoints[i1], curvePoints[i2], curvePoints[i3], m_fPathTime);
		float nextT = min(m_fPathTime + 0.01f, 1.0f);
		XMFLOAT3 nextPos = CatmullRom(curvePoints[i0], curvePoints[i1], curvePoints[i2], curvePoints[i3], nextT);

		// ���� ���� ���
		XMFLOAT3 forward = Vector3::Normalize(Vector3::Subtract(nextPos, currPos));
		XMFLOAT3 right = Vector3::Normalize(Vector3::CrossProduct(XMFLOAT3(0, 1, 0), forward));
		XMFLOAT3 up = Vector3::CrossProduct(forward, right);

		m_xmf3Look = forward;
		m_xmf3Right = right;
		m_xmf3Up = up;

		// ��ġ ����
		SetPosition(currPos);

		// ���� ��� ����
		m_xmf4x4World._11 = m_xmf3Right.x;   m_xmf4x4World._12 = m_xmf3Right.y;   m_xmf4x4World._13 = m_xmf3Right.z;   m_xmf4x4World._14 = 0.0f;
		m_xmf4x4World._21 = m_xmf3Up.x;      m_xmf4x4World._22 = m_xmf3Up.y;      m_xmf4x4World._23 = m_xmf3Up.z;      m_xmf4x4World._24 = 0.0f;
		m_xmf4x4World._31 = m_xmf3Look.x;    m_xmf4x4World._32 = m_xmf3Look.y;    m_xmf4x4World._33 = m_xmf3Look.z;    m_xmf4x4World._34 = 0.0f;
		m_xmf4x4World._41 = m_xmf3Position.x; m_xmf4x4World._42 = m_xmf3Position.y; m_xmf4x4World._43 = m_xmf3Position.z; m_xmf4x4World._44 = 1.0f;
	}
	else
	{
		eFlag = true; // ��� ���� �� ���� ó��
	}

	// �߰� ��ó�� �� ī�޶� ������Ʈ
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fElapsedTime);

	DWORD nCameraMode = m_pCamera->GetMode();
	if (nCameraMode == THIRD_PERSON_CAMERA)
		m_pCamera->Update(m_xmf3Position, fElapsedTime);

	if (m_pCameraUpdatedContext)
		OnCameraUpdateCallback(fElapsedTime);

	if (nCameraMode == THIRD_PERSON_CAMERA)
		m_pCamera->SetLookAt(m_xmf3Position);

	m_pCamera->RegenerateViewMatrix();
}


//ī�޶� ������ �� ȣ��Ǵ� �Լ��̴�. nNewCameraMode�� ���� ������ ī�޶� ����̴�. 
CCamera* CCartPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		//�÷��̾��� Ư���� 1��Ī ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�.
		SetFriction(200.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		//�÷��̾��� Ư���� �����̽�-�� ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�.
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(400.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		//�÷��̾��� Ư���� 3��Ī ī�޶� ��忡 �°� �����Ѵ�. ���� ȿ���� ī�޶� �������� �����Ѵ�.
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�. ���� 0.25f ��ſ� 0.0f�� 1.0f�� ������ ����� ���ϱ� �ٶ���.
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	//�÷��̾ �ð��� ����� ���� ����(��ġ�� ������ ����: �ӵ�, ������, �߷� ���� ó��)�Ѵ�.
	Update(fTimeElapsed);
	return(m_pCamera);
}

// CTankTurret
CTank::CTank(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	CTankBodyMesh* pTankBodyMesh = new CTankBodyMesh(pd3dDevice, pd3dCommandList);
	SetMesh(pTankBodyMesh);

	CCubeMeshDiffused* pBulletMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 0.5f, 2.0f, 0.5f);
	for (int i = 0; i < BULLETS; ++i)
	{
		m_ppBullets[i] = new CBulletObject(m_fBulletEffectiveRange);
		m_ppBullets[i]->SetMesh(pBulletMesh);
		m_ppBullets[i]->SetActive(false);
		m_ppBullets[i]->SetMovingSpeed(100.f);
	}

	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));

	CPlayerShader* pShader = new CPlayerShader();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}
CTank::~CTank()
{
}

void CTank::OnPrepareRender()
{
	CPlayer::OnPrepareRender();
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f,
		0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

CCamera* CTank::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return(m_pCamera);
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		//�÷��̾��� Ư���� 1��Ī ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�.
		SetFriction(200.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case SPACESHIP_CAMERA:
		//�÷��̾��� Ư���� �����̽�-�� ī�޶� ��忡 �°� �����Ѵ�. �߷��� �������� �ʴ´�.
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(400.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	case THIRD_PERSON_CAMERA:
		//�÷��̾��� Ư���� 3��Ī ī�޶� ��忡 �°� �����Ѵ�. ���� ȿ���� ī�޶� �������� �����Ѵ�.
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		//3��Ī ī�޶��� ���� ȿ���� �����Ѵ�. ���� 0.25f ��ſ� 0.0f�� 1.0f�� ������ ����� ���ϱ� �ٶ���.
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT, 0.0f,
			1.0f);
		m_pCamera->SetScissorRect(0, 0, FRAME_BUFFER_WIDTH, FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	//�÷��̾ �ð��� ����� ���� ����(��ġ�� ������ ����: �ӵ�, ������, �߷� ���� ó��)�Ѵ�.
	Update(fTimeElapsed);
	return(m_pCamera);
}

void CTank::FireBullet(CGameObject* pLockedObject)
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
		pBullet->SetActive(true);

		char buf[128];
		sprintf_s(buf, "fireDir: %.2f %.2f %.2f\n", fireDir.x, fireDir.y, fireDir.z);
		OutputDebugStringA(buf);

		if (pLockedObject)
		{
			pBullet->m_pLockedObject = pLockedObject;
		}

	}

}

void CTank::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject::Render(pd3dCommandList, pCamera);

	for (int i = 0; i < BULLETS; ++i)
	{
		if (m_ppBullets[i]->m_bActive)
		{
			m_ppBullets[i]->Render(pd3dCommandList, pCamera);
		}
	}
}

void CTank::Update(float fElapsedTime)
{

	CPlayer::Update(fElapsedTime);

	for (int i = 0; i < BULLETS; ++i) {
		if (m_ppBullets[i]->m_bActive) {
			m_ppBullets[i]->Animate(fElapsedTime);
		}
	}
}

void CTank::CheckBulletCollision(CGameObject** ppEnemies, int nEnemies)
{
	for (int i = 0; i < BULLETS; ++i)
	{
		CBulletObject* pBullet = m_ppBullets[i];
		if (!pBullet->m_bActive) continue;

		// �Ѿ��� �ٿ�� �ڽ��� ������ǥ��� ��ȯ
		BoundingOrientedBox bulletBox = pBullet->m_pMesh->GetBoundingBox();
		bulletBox.Transform(bulletBox, XMLoadFloat4x4(&pBullet->m_xmf4x4World));

		for (int j = 0; j < nEnemies; ++j)
		{
			CGameObject* pEnemy = ppEnemies[j];
			if (!pEnemy->m_bActive) continue;

			BoundingOrientedBox enemyBox = pEnemy->m_pMesh->GetBoundingBox();
			enemyBox.Transform(enemyBox, XMLoadFloat4x4(&pEnemy->m_xmf4x4World));

			if (bulletBox.Intersects(enemyBox))
			{
				pBullet->Reset();
				pEnemy->SetActive(false);
				OutputDebugStringA("�浹 ����!\n");
			}
		}
	}
}
