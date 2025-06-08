#pragma once
#include "stdafx.h"
#include "Camera.h"
//������ ǥ���ϱ� ���� Ŭ������ �����Ѵ�.
class CVertex
{
protected:
	//������ ��ġ �����̴�(��� ������ �ּ��� ��ġ ���͸� ������ �Ѵ�).
	XMFLOAT3 m_xmf3Position;
public:
	CVertex() { m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); }
	CVertex(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	~CVertex() {}
};
class CDiffusedVertex : public CVertex
{
protected:
	//������ �����̴�.
	XMFLOAT4 m_xmf4Diffuse;
public:
	CDiffusedVertex() {
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f); m_xmf4Diffuse =
			XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
	}
	CDiffusedVertex(float x, float y, float z, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position =
			XMFLOAT3(x, y, z); m_xmf4Diffuse = xmf4Diffuse;
	}
	CDiffusedVertex(XMFLOAT3 xmf3Position, XMFLOAT4 xmf4Diffuse) {
		m_xmf3Position =
			xmf3Position; m_xmf4Diffuse = xmf4Diffuse;
	}
	~CDiffusedVertex() {}
};

class CMesh
{
public:
	CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CMesh();
private:
	int m_nReferences = 0;
public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	void ReleaseUploadBuffers();
protected:
	ID3D12Resource* m_pd3dVertexBuffer = NULL;
	ID3D12Resource* m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW m_d3dVertexBufferView;
	D3D12_PRIMITIVE_TOPOLOGY m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT m_nSlot = 0;
	UINT m_nVertices = 0;
	UINT m_nStride = 0;
	UINT m_nOffset = 0;

	int							targetStage;
	bool						beSheild = false;
public:
	BoundingOrientedBox	m_xmBoundingBox = BoundingOrientedBox();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	int CheckRayIntersection(XMFLOAT3& xmRayPosition, XMFLOAT3& xmRayDirection, float* pfNearHitDistance);
	//�ε��� ���۸� ���� ����
	int getTargetStage() { return targetStage; }
	BoundingOrientedBox GetBoundingBox() { return m_xmBoundingBox; }
protected:
	ID3D12Resource* m_pd3dIndexBuffer = NULL;
	ID3D12Resource* m_pd3dIndexUploadBuffer = NULL;
	/*�ε��� ����(�ε����� �迭)�� �ε��� ���۸� ���� ���ε� ���ۿ� ���� �������̽� �������̴�. �ε��� ���۴� ����
   ����(�迭)�� ���� �ε����� ������.*/
	D3D12_INDEX_BUFFER_VIEW m_d3dIndexBufferView;
	UINT m_nIndices = 0;
	//�ε��� ���ۿ� ���ԵǴ� �ε����� �����̴�. 
	UINT m_nStartIndex = 0;
	//�ε��� ���ۿ��� �޽��� �׸��� ���� ���Ǵ� ���� �ε����̴�. 
	int m_nBaseVertex = 0;
	//�ε��� ������ �ε����� ������ �ε����̴�. 

	CDiffusedVertex* m_pVertices = NULL;
	UINT* m_pnIndices = NULL;
};

class CTriangleMesh : public CMesh
{
public:
	CTriangleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CTriangleMesh() {}
};

class CCubeMeshDiffused : public CMesh
{
public:
	//������ü�� ����, ����, ������ ���̸� �����Ͽ� ������ü �޽��� �����Ѵ�.
	CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth = 2.0f, float fHeight = 2.0f, float fDepth = 2.0f);
	virtual ~CCubeMeshDiffused();
};


//�����ϱ�12
class CAirplaneMeshDiffused : public CMesh
{
public:
	CAirplaneMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
		* pd3dCommandList, float fWidth = 20.0f, float fHeight = 20.0f, float fDepth = 4.0f,
		XMFLOAT4 xmf4Color = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f));
	virtual ~CAirplaneMeshDiffused();
};

class CTextMesh : public CMesh
{
public:
	template <size_t N1, size_t N2, size_t N3>

	CTextMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int target = 0,
		std::array<bool, N1> text = true,
		std::array<float, N2> cx = 0,
		std::array<float, N3> cy = 0
	)
		: CMesh(pd3dDevice, pd3dCommandList)
	{
		float fW = 0.5f;
		float fH = 0.5f;
		float fD = 0.5f;

		int cnt = 0;

		targetStage = target;

		int cubeCount = std::count(text.begin(), text.end(), true);

		m_nVertices = 8 * cubeCount;
		m_nStride = sizeof(CDiffusedVertex);
		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		m_pVertices = new CDiffusedVertex[m_nVertices];
		for (int i = 0; i < text.size(); ++i) {
			int x = i % cx.size();
			int y = i / cx.size();
			if (text[i]) {
				m_pVertices[cnt] = CDiffusedVertex(XMFLOAT3(cx[x] - fW, cy[y] + fH, -fD), RANDOM_COLOR);
				m_pVertices[cnt + 1] = CDiffusedVertex(XMFLOAT3(cx[x] + fW, cy[y] + fH, -fD), RANDOM_COLOR);
				m_pVertices[cnt + 2] = CDiffusedVertex(XMFLOAT3(cx[x] + fW, cy[y] + fH, +fD), RANDOM_COLOR);
				m_pVertices[cnt + 3] = CDiffusedVertex(XMFLOAT3(cx[x] - fW, cy[y] + fH, +fD), RANDOM_COLOR);
				m_pVertices[cnt + 4] = CDiffusedVertex(XMFLOAT3(cx[x] - fW, cy[y] - fH, -fD), RANDOM_COLOR);
				m_pVertices[cnt + 5] = CDiffusedVertex(XMFLOAT3(cx[x] + fW, cy[y] - fH, -fD), RANDOM_COLOR);
				m_pVertices[cnt + 6] = CDiffusedVertex(XMFLOAT3(cx[x] + fW, cy[y] - fH, +fD), RANDOM_COLOR);
				m_pVertices[cnt + 7] = CDiffusedVertex(XMFLOAT3(cx[x] - fW, cy[y] - fH, +fD), RANDOM_COLOR);

				cnt += 8;
			}
		}
		cnt = 0;

		m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pVertices,
			m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
		m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
		m_d3dVertexBufferView.StrideInBytes = m_nStride;
		m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

		m_nIndices = 36 * cubeCount;
		int cnt2 = 0;
		m_pnIndices = new UINT[m_nIndices];
		for (int i = 0; i < cubeCount; ++i) {
			//�� �ո�(Front) �簢���� ���� �ﰢ��
			m_pnIndices[cnt] = cnt2 + 3; m_pnIndices[cnt + 1] = cnt2 + 1; m_pnIndices[cnt + 2] = cnt2;
			//�� �ո�(Front) �簢���� �Ʒ��� �ﰢ��
			m_pnIndices[cnt + 3] = cnt2 + 2; m_pnIndices[cnt + 4] = cnt2 + 1; m_pnIndices[cnt + 5] = cnt2 + 3;
			//�� ����(Top) �簢���� ���� �ﰢ��
			m_pnIndices[cnt + 6] = cnt2 + 0; m_pnIndices[cnt + 7] = cnt2 + 5; m_pnIndices[cnt + 8] = cnt2 + 4;
			//�� ����(Top) �簢���� �Ʒ��� �ﰢ��
			m_pnIndices[cnt + 9] = cnt2 + 1; m_pnIndices[cnt + 10] = cnt2 + 5; m_pnIndices[cnt + 11] = cnt2;
			//�� �޸�(Back) �簢���� ���� �ﰢ��
			m_pnIndices[cnt + 12] = cnt2 + 3; m_pnIndices[cnt + 13] = cnt2 + 4; m_pnIndices[cnt + 14] = cnt2 + 7;
			//�� �޸�(Back) �簢���� �Ʒ��� �ﰢ��
			m_pnIndices[cnt + 15] = cnt2; m_pnIndices[cnt + 16] = cnt2 + 4; m_pnIndices[cnt + 17] = cnt2 + 3;
			//�� �Ʒ���(Bottom) �簢���� ���� �ﰢ��
			m_pnIndices[cnt + 18] = cnt2 + 1; m_pnIndices[cnt + 19] = cnt2 + 6; m_pnIndices[cnt + 20] = cnt2 + 5;
			//�� �Ʒ���(Bottom) �簢���� �Ʒ��� �ﰢ��
			m_pnIndices[cnt + 21] = cnt2 + 2; m_pnIndices[cnt + 22] = cnt2 + 6; m_pnIndices[cnt + 23] = cnt2 + 1;
			//�� ����(Left) �簢���� ���� �ﰢ��
			m_pnIndices[cnt + 24] = cnt2 + 2; m_pnIndices[cnt + 25] = cnt2 + 7; m_pnIndices[cnt + 26] = cnt2 + 6;
			//�� ����(Left) �簢���� �Ʒ��� �ﰢ��
			m_pnIndices[cnt + 27] = cnt2 + 3; m_pnIndices[cnt + 28] = cnt2 + 7; m_pnIndices[cnt + 29] = cnt2 + 2;
			//�� ����(Right) �簢���� ���� �ﰢ��
			m_pnIndices[cnt + 30] = cnt2 + 6; m_pnIndices[cnt + 31] = cnt2 + 4; m_pnIndices[cnt + 32] = cnt2 + 5;
			//�� ����(Right) �簢���� �Ʒ��� �ﰢ��
			m_pnIndices[cnt + 33] = cnt2 + 7; m_pnIndices[cnt + 34] = cnt2 + 4; m_pnIndices[cnt + 35] = cnt2 + 6;
			//�ε��� ���۸� �����Ѵ�.

			cnt += 36;
			cnt2 += 8;
		}

		m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnIndices,
			sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
			&m_pd3dIndexUploadBuffer);
		//�ε��� ���� �並 �����Ѵ�.
		m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
		m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
		m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

		m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f),
			XMFLOAT3(cx[cx.size() - 1], cy[0], fD),
			XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	virtual ~CTextMesh() {}
};


class CCartMesh : public CMesh
{
public:
	CCartMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CCartMesh() {}
};

class CRailMesh : public CMesh
{
public:
	CRailMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		float width = 0.3f, float height = 0.3f, float depth = 1.0f);
	virtual ~CRailMesh() {}
};

class CTankBodyMesh : public CMesh
{
public:
	CTankBodyMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CTankBodyMesh() {}
};

class CTankTurretMesh : public CMesh
{
public:
	CTankTurretMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CTankTurretMesh() {}
};

class CEnemyTankMesh : public CMesh
{
public:
	CEnemyTankMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CEnemyTankMesh() {}
};