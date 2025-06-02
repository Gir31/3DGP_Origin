#include "stdafx.h"
#include "Mesh.h"
#include "GraphicsPipeline.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CPolygon::CPolygon(int nVertices, COLORREF color)
{
	m_nVertices = nVertices;
	m_pVertices = new CVertex[nVertices];

	polygonColor = color;
}

CPolygon::~CPolygon()
{
	if (m_pVertices) delete[] m_pVertices;
}

void CPolygon::SetVertex(int nIndex, CVertex& vertex)
{
	if ((0 <= nIndex) && (nIndex < m_nVertices) && m_pVertices)
	{
		m_pVertices[nIndex] = vertex;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMesh::CMesh(int nPolygons)
{
	m_nPolygons = nPolygons;
	m_ppPolygons = new CPolygon*[nPolygons];
}

CMesh::~CMesh()
{
	if (m_ppPolygons)
	{
		for (int i = 0; i < m_nPolygons; i++) if (m_ppPolygons[i]) delete m_ppPolygons[i];
		delete[] m_ppPolygons;
	}
}

int CMesh::getTargetStage()
{
	return targetStage;
}

void CMesh::SetPolygon(int nIndex, CPolygon *pPolygon)
{
	if ((0 <= nIndex) && (nIndex < m_nPolygons)) {
		m_ppPolygons[nIndex] = pPolygon;
	}
}

BOOL CMesh::RayIntersectionByTriangle(XMVECTOR& xmRayOrigin, XMVECTOR& xmRayDirection, XMVECTOR v0, XMVECTOR v1, XMVECTOR v2, float* pfNearHitDistance)
{
	float fHitDistance;
	BOOL bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0, v1, v2, fHitDistance);
	if (bIntersected && (fHitDistance < *pfNearHitDistance)) *pfNearHitDistance = fHitDistance;

	return(bIntersected);
}

int CMesh::CheckRayIntersection(XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection, float* pfNearHitDistance)
{
	int nIntersections = 0;
	bool bIntersected = m_xmOOBB.Intersects(xmvPickRayOrigin, xmvPickRayDirection, *pfNearHitDistance);
	if (bIntersected)
	{
		for (int i = 0; i < m_nPolygons; i++)
		{
			switch (m_ppPolygons[i]->m_nVertices)
			{
			case 3:
			{
				XMVECTOR v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				XMVECTOR v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[1].m_xmf3Position));
				XMVECTOR v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				break;
			}
			case 4:
			{
				XMVECTOR v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				XMVECTOR v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[1].m_xmf3Position));
				XMVECTOR v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				BOOL bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				v0 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[0].m_xmf3Position));
				v1 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[2].m_xmf3Position));
				v2 = XMLoadFloat3(&(m_ppPolygons[i]->m_pVertices[3].m_xmf3Position));
				bIntersected = RayIntersectionByTriangle(xmvPickRayOrigin, xmvPickRayDirection, v0, v1, v2, pfNearHitDistance);
				if (bIntersected) nIntersections++;
				break;
			}
			}
		}
	}
	return(nIntersections);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CCubeMesh::CCubeMesh(float fWidth, float fHeight, float fDepth) : CMesh(6)
{
	float fHalfWidth = fWidth * 0.5f;
	float fHalfHeight = fHeight * 0.5f;
	float fHalfDepth = fDepth * 0.5f;

	CPolygon *pFrontFace = new CPolygon(4, RGB(0, 0, 0));
	pFrontFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pFrontFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(0, pFrontFace);

	CPolygon *pTopFace = new CPolygon(4, RGB(0, 0, 0));
	pTopFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pTopFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pTopFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	SetPolygon(1, pTopFace);

	CPolygon *pBackFace = new CPolygon(4, RGB(0, 0, 0));
	pBackFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(2, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pBackFace->SetVertex(3, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	SetPolygon(2, pBackFace);

	CPolygon *pBottomFace = new CPolygon(4, RGB(0, 0, 0));
	pBottomFace->SetVertex(0, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(1, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	pBottomFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pBottomFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(3, pBottomFace);

	CPolygon *pLeftFace = new CPolygon(4, RGB(0, 0, 0));
	pLeftFace->SetVertex(0, CVertex(-fHalfWidth, +fHalfHeight, +fHalfDepth));
	pLeftFace->SetVertex(1, CVertex(-fHalfWidth, +fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(2, CVertex(-fHalfWidth, -fHalfHeight, -fHalfDepth));
	pLeftFace->SetVertex(3, CVertex(-fHalfWidth, -fHalfHeight, +fHalfDepth));
	SetPolygon(4, pLeftFace);

	CPolygon *pRightFace = new CPolygon(4, RGB(0, 0, 0));
	pRightFace->SetVertex(0, CVertex(+fHalfWidth, +fHalfHeight, -fHalfDepth));
	pRightFace->SetVertex(1, CVertex(+fHalfWidth, +fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(2, CVertex(+fHalfWidth, -fHalfHeight, +fHalfDepth));
	pRightFace->SetVertex(3, CVertex(+fHalfWidth, -fHalfHeight, -fHalfDepth));
	SetPolygon(5, pRightFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fHalfWidth, fHalfHeight, fHalfDepth), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CCursorMesh::CCursorMesh(float fWidth, float fHeight, float fDepth) : CMesh(1)
{
	float fx = fWidth*0.5f, fy = fHeight*0.5f, fz = fDepth*0.5f;

	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy - y3)) / x1)*x2 + (fy - y3);
	int i = 0;

	//Upper Plane
	CPolygon *pFace = new CPolygon(3, RGB(0, 0, 0));
	pFace->SetVertex(0, CVertex(0.f, 0.f, 0.f));
	pFace->SetVertex(1, CVertex(0.f, 0.f, 0.f));
	pFace->SetVertex(2, CVertex(0.f, 0.f, 0.f));
	SetPolygon(i++, pFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(fx, fy, fz), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

CCartMesh::CCartMesh() : CMesh(71)
{
	int i = 0;

	COLORREF bodyColor		= RGB(0, 100, 0);
	COLORREF innerColor		= RGB(166, 73, 53);
	COLORREF seatColor		= RGB(211, 178, 129);
	COLORREF barColor		= RGB(50, 50, 50);
	COLORREF frameColor		= RGB(100, 100, 100);

	// Cart 바닥				| 사용 Mesh : 6
	{
		float x1 = -4.0f;	float x2 = 4.0f;
		float y1 = 10.0f;	float y2 = -10.0f;  // 앞뒤 (Y축)
		float z1 = 3.0f;	float z2 = 2.0f;  // 아래위 (Z축)
		COLORREF color = RGB(0, 0, 0);

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, color);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 아랫면 (z1)
		CPolygon* bottom = new CPolygon(4, color);
		bottom->SetVertex(0, CVertex({ x2, y1, z1 }));
		bottom->SetVertex(1, CVertex({ x1, y1, z1 }));
		bottom->SetVertex(2, CVertex({ x1, y2, z1 }));
		bottom->SetVertex(3, CVertex({ x2, y2, z1 }));
		SetPolygon(i++, bottom);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, color);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, color);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, color);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, color);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 뒤쪽 벽				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = -8.8f;	float y2 = -9.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = -2.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, frameColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, innerColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, bodyColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, bodyColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 오른쪽 벽			| 사용 Mesh : 5
	{
		float x1 = 2.8f;	float x2 = 3.8f;
		float y1 = 9.8f;	float y2 = -9.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = 0.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, frameColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, bodyColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, innerColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, bodyColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 왼쪽 벽				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = -2.8f;
		float y1 = 9.8f;	float y2 = -9.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = 0.0f;  // 아래위 (Z축)
		
		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, frameColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, bodyColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, bodyColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, innerColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 앞쪽 벽				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = 9.8f;	float y2 = 8.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = 0.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, frameColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, bodyColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, bodyColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, bodyColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// Cart 본넷				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = 9.8f;	float y2 = 3.8f;  // 앞뒤 (Y축)
		float z1 = 0.0f;	float z2 = -1.0f;  // 아래위 (Z축)

		// 아래면
		CPolygon* bottom = new CPolygon(4, bodyColor);
		bottom->SetVertex(0, CVertex({ x1, y1, z1 }));
		bottom->SetVertex(1, CVertex({ x2, y1, z1 }));
		bottom->SetVertex(2, CVertex({ x2, y2, z1 }));
		bottom->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, bottom);

		// 뒷면
		CPolygon* back = new CPolygon(4, bodyColor);
		back->SetVertex(0, CVertex({ x1, y2, z1 }));
		back->SetVertex(1, CVertex({ x2, y2, z1 }));
		back->SetVertex(2, CVertex({ x2, y2, z2 }));
		back->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, back);

		// 윗면
		CPolygon* top = new CPolygon(4, bodyColor);
		top->SetVertex(0, CVertex({ x1, y1, z1 }));
		top->SetVertex(1, CVertex({ x2, y1, z1 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 오른쪽
		CPolygon* right = new CPolygon(3, bodyColor);
		right->SetVertex(0, CVertex({ x2, y2, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		SetPolygon(i++, right);

		// 왼쪽
		CPolygon* left = new CPolygon(3, bodyColor);
		left->SetVertex(0, CVertex({ x1, y2, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z1 }));
		left->SetVertex(2, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);
	}
	// 앞쪽 좌석
	{
		float x1 = -2.8f;	float x2 = 2.8f;
		float y1 = -2.0f;	float y2 = -3.0f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = -1.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 앞쪽 좌석 머리 1
	{
		float x1 = -1.9f;	float x2 = -0.9f;
		float y1 = -2.2f;	float y2 = -2.8f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 앞쪽 좌석 머리 2
	{
		float x1 = 0.9f;	float x2 = 1.9f;
		float y1 = -2.2f;	float y2 = -2.8f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 앞쪽 안전 바
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = 1.0f;	float y2 = 0.5f;  // 앞뒤 (Y축)
		float z1 = 0.0f;	float z2 = -0.5f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, barColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, barColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, barColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, barColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, barColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 뒤쪽 좌석
	{
		float x1 = -2.8f;	float x2 = 2.8f;
		float y1 = -7.8f;	float y2 = -8.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = -1.0f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 뒤쪽 좌석 머리 1
	{
		float x1 = -1.9f;	float x2 = -0.9f;
		float y1 = -8.0f;	float y2 = -8.6f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 뒤쪽 좌석 머리 2
	{
		float x1 = 0.9f;	float x2 = 1.9f;
		float y1 = -8.0f;	float y2 = -8.6f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, seatColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, seatColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, seatColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, seatColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, seatColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}
	// 뒤쪽 안전 바
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = -4.8f;	float y2 = -5.3f;  // 앞뒤 (Y축)
		float z1 = 0.0f;	float z2 = -0.5f;  // 아래위 (Z축)

		// 윗면 (z2)
		CPolygon* top = new CPolygon(4, barColor);
		top->SetVertex(0, CVertex({ x1, y1, z2 }));
		top->SetVertex(1, CVertex({ x2, y1, z2 }));
		top->SetVertex(2, CVertex({ x2, y2, z2 }));
		top->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, top);

		// 앞면 (+Y)
		CPolygon* front = new CPolygon(4, barColor);
		front->SetVertex(0, CVertex({ x2, y1, z2 }));
		front->SetVertex(1, CVertex({ x1, y1, z2 }));
		front->SetVertex(2, CVertex({ x1, y1, z1 }));
		front->SetVertex(3, CVertex({ x2, y1, z1 }));
		SetPolygon(i++, front);

		// 뒷면 (+Y)
		CPolygon* back = new CPolygon(4, barColor);
		back->SetVertex(0, CVertex({ x1, y2, z2 }));
		back->SetVertex(1, CVertex({ x2, y2, z2 }));
		back->SetVertex(2, CVertex({ x2, y2, z1 }));
		back->SetVertex(3, CVertex({ x1, y2, z1 }));
		SetPolygon(i++, back);

		// 왼쪽 (-X)
		CPolygon* left = new CPolygon(4, barColor);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y2, z2 }));
		left->SetVertex(2, CVertex({ x1, y2, z1 }));
		left->SetVertex(3, CVertex({ x1, y1, z1 }));
		SetPolygon(i++, left);

		// 오른쪽 (+X)
		CPolygon* right = new CPolygon(4, barColor);
		right->SetVertex(0, CVertex({ x2, y1, z2 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y2, z1 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);
	}

	// [마무리] 바운딩 박스 설정
	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

CRailMesh::CRailMesh(float width, float height, float depth) : CMesh(6)
{
	float w = width * 0.5f;
	float h = height * 0.5f;
	float d = depth * 0.5f;

	CPolygon* pFrontFace = new CPolygon(4, RGB(0, 0, 0));
	pFrontFace->SetVertex(0, CVertex(-w, +h, -d));
	pFrontFace->SetVertex(1, CVertex(+w, +h, -d));
	pFrontFace->SetVertex(2, CVertex(+w, -h, -d));
	pFrontFace->SetVertex(3, CVertex(-w, -h, -d));
	SetPolygon(0, pFrontFace);

	CPolygon* pTopFace = new CPolygon(4, RGB(0, 0, 0));
	pTopFace->SetVertex(0, CVertex(-w, +h, +d));
	pTopFace->SetVertex(1, CVertex(+w, +h, +d));
	pTopFace->SetVertex(2, CVertex(+w, +h, -d));
	pTopFace->SetVertex(3, CVertex(-w, +h, -d));
	SetPolygon(1, pTopFace);

	CPolygon* pBackFace = new CPolygon(4, RGB(0, 0, 0));
	pBackFace->SetVertex(0, CVertex(-w, -h, +d));
	pBackFace->SetVertex(1, CVertex(+w, -h, +d));
	pBackFace->SetVertex(2, CVertex(+w, +h, +d));
	pBackFace->SetVertex(3, CVertex(-w, +h, +d));
	SetPolygon(2, pBackFace);

	CPolygon* pBottomFace = new CPolygon(4, RGB(0, 0, 0));
	pBottomFace->SetVertex(0, CVertex(-w, -h, -d));
	pBottomFace->SetVertex(1, CVertex(+w, -h, -d));
	pBottomFace->SetVertex(2, CVertex(+w, -h, +d));
	pBottomFace->SetVertex(3, CVertex(-w, -h, +d));
	SetPolygon(3, pBottomFace);

	CPolygon* pLeftFace = new CPolygon(4, RGB(0, 0, 0));
	pLeftFace->SetVertex(0, CVertex(-w, +h, +d));
	pLeftFace->SetVertex(1, CVertex(-w, +h, -d));
	pLeftFace->SetVertex(2, CVertex(-w, -h, -d));
	pLeftFace->SetVertex(3, CVertex(-w, -h, +d));
	SetPolygon(4, pLeftFace);

	CPolygon* pRightFace = new CPolygon(4, RGB(0, 0, 0));
	pRightFace->SetVertex(0, CVertex(+w, +h, -d));
	pRightFace->SetVertex(1, CVertex(+w, +h, +d));
	pRightFace->SetVertex(2, CVertex(+w, -h, +d));
	pRightFace->SetVertex(3, CVertex(+w, -h, -d));
	SetPolygon(5, pRightFace);

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(w, h, d), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

CTankBodyMesh::CTankBodyMesh() : CMesh(19)
{
	int i = 0;

	
	{ // 몸체
		float x1 = -4.f;	float x2 = 4.f;		float x3 = -5.f;	float x4 = 5.f;
		float y1 = 10.f;	float y2 = 9.f;		float y3 = 6.f;		float y4 = -10.f;
		float z1 = 0.f;		float z2 = 1.5f;		float z3 = 2.f;

		COLORREF color = RGB(107, 142, 35);

		// 윗면
		CPolygon* top = new CPolygon(4, color);
		top->SetVertex(0, CVertex({ x1, y4, z1 }));
		top->SetVertex(1, CVertex({ x1, y3, z1 }));
		top->SetVertex(2, CVertex({ x2, y3, z1 }));
		top->SetVertex(3, CVertex({ x2, y4, z1 }));
		SetPolygon(i++, top);

		CPolygon* backTop = new CPolygon(4, color);
		backTop->SetVertex(0, CVertex({ x1, y4, z1 }));
		backTop->SetVertex(1, CVertex({ x2, y4, z1 }));
		backTop->SetVertex(2, CVertex({ x4, y4, z2 }));
		backTop->SetVertex(3, CVertex({ x3, y4, z2 }));
		SetPolygon(i++, backTop);

		CPolygon* backBottom = new CPolygon(4, color);
		backBottom->SetVertex(0, CVertex({ x3, y4, z2 }));
		backBottom->SetVertex(1, CVertex({ x4, y4, z2 }));
		backBottom->SetVertex(2, CVertex({ x4, y4, z3 }));
		backBottom->SetVertex(3, CVertex({ x3, y4, z3 }));
		SetPolygon(i++, backBottom);


		CPolygon* rightTop = new CPolygon(4, color);
		rightTop->SetVertex(0, CVertex({ x4, y4, z2 }));
		rightTop->SetVertex(1, CVertex({ x2, y4, z1 }));
		rightTop->SetVertex(2, CVertex({ x2, y3, z1 }));
		rightTop->SetVertex(3, CVertex({ x4, y1, z2 }));
		SetPolygon(i++, rightTop);

		CPolygon* rightBottom = new CPolygon(4, color);
		rightBottom->SetVertex(0, CVertex({ x4, y4, z3 }));
		rightBottom->SetVertex(1, CVertex({ x4, y4, z2 }));
		rightBottom->SetVertex(2, CVertex({ x4, y1, z2 }));
		rightBottom->SetVertex(3, CVertex({ x4, y2, z3 }));
		SetPolygon(i++, rightBottom);

		CPolygon* leftTop = new CPolygon(4, color);
		leftTop->SetVertex(0, CVertex({ x3, y1, z2 }));
		leftTop->SetVertex(1, CVertex({ x1, y3, z1 }));
		leftTop->SetVertex(2, CVertex({ x1, y4, z1 }));
		leftTop->SetVertex(3, CVertex({ x3, y4, z2 }));
		SetPolygon(i++, leftTop);

		CPolygon* leftBottom = new CPolygon(4, color);
		leftBottom->SetVertex(0, CVertex({ x3, y2, z3 }));
		leftBottom->SetVertex(1, CVertex({ x3, y1, z2 }));
		leftBottom->SetVertex(2, CVertex({ x3, y4, z2 }));
		leftBottom->SetVertex(3, CVertex({ x3, y4, z3 }));
		SetPolygon(i++, leftBottom);

		CPolygon* frontTop = new CPolygon(4, color);
		frontTop->SetVertex(0, CVertex({ x4, y1, z2 }));
		frontTop->SetVertex(1, CVertex({ x2, y3, z1 }));
		frontTop->SetVertex(2, CVertex({ x1, y3, z1 }));
		frontTop->SetVertex(3, CVertex({ x3, y1, z2 }));
		SetPolygon(i++, frontTop);

		CPolygon* frontBottom = new CPolygon(4, color);
		frontBottom->SetVertex(0, CVertex({ x4, y2, z3 }));
		frontBottom->SetVertex(1, CVertex({ x4, y1, z2 }));
		frontBottom->SetVertex(2, CVertex({ x3, y1, z2 }));
		frontBottom->SetVertex(3, CVertex({ x3, y2, z3 }));
		SetPolygon(i++, frontBottom);
	}
	{ // 오른쪽 바퀴
		float x1 = 3.f;		float x2 = 4.8f;
		float y1 = 8.8f;	float y2 = 8.f;		float y3 = -9.f;		float y4 = -9.8f;
		float z1 = 2.f;		float z2 = 2.5f;	float z3 = 3.f;

		COLORREF color = RGB(0, 0, 0);

		CPolygon* bottom = new CPolygon(4, color);
		bottom->SetVertex(0, CVertex({ x2, y2, z3 }));
		bottom->SetVertex(1, CVertex({ x1, y2, z3 }));
		bottom->SetVertex(2, CVertex({ x1, y3, z3 }));
		bottom->SetVertex(3, CVertex({ x2, y3, z3 }));
		SetPolygon(i++, bottom);

		CPolygon* frontBottom = new CPolygon(4, color);
		frontBottom->SetVertex(0, CVertex({ x2, y1, z2 }));
		frontBottom->SetVertex(1, CVertex({ x1, y1, z2 }));
		frontBottom->SetVertex(2, CVertex({ x1, y2, z3 }));
		frontBottom->SetVertex(3, CVertex({ x2, y2, z3 }));
		SetPolygon(i++, frontBottom);

		CPolygon* front = new CPolygon(4, color);
		front->SetVertex(0, CVertex({ x2, y1, z1 }));
		front->SetVertex(1, CVertex({ x1, y1, z1 }));
		front->SetVertex(2, CVertex({ x1, y1, z2 }));
		front->SetVertex(3, CVertex({ x2, y1, z2 }));
		SetPolygon(i++, front);

		CPolygon* backBottom = new CPolygon(4, color);
		backBottom->SetVertex(0, CVertex({ x1, y4, z2 }));
		backBottom->SetVertex(1, CVertex({ x2, y4, z2 }));
		backBottom->SetVertex(2, CVertex({ x2, y3, z3 }));
		backBottom->SetVertex(3, CVertex({ x1, y3, z3 }));
		SetPolygon(i++, backBottom);

		CPolygon* back = new CPolygon(4, color);
		back->SetVertex(0, CVertex({ x1, y4, z1 }));
		back->SetVertex(1, CVertex({ x2, y4, z1 }));
		back->SetVertex(2, CVertex({ x2, y4, z2 }));
		back->SetVertex(3, CVertex({ x1, y4, z2 }));
		SetPolygon(i++, back);
	}
	{ // 왼쪽 바퀴
		float x1 = -4.8f;		float x2 = -3.f;
		float y1 = 8.8f;	float y2 = 8.f;		float y3 = -9.f;		float y4 = -9.8f;
		float z1 = 2.f;		float z2 = 2.5f;	float z3 = 3.f;

		COLORREF color = RGB(0, 0, 0);

		CPolygon* bottom = new CPolygon(4, color);
		bottom->SetVertex(0, CVertex({ x2, y2, z3 }));
		bottom->SetVertex(1, CVertex({ x1, y2, z3 }));
		bottom->SetVertex(2, CVertex({ x1, y3, z3 }));
		bottom->SetVertex(3, CVertex({ x2, y3, z3 }));
		SetPolygon(i++, bottom);

		CPolygon* frontBottom = new CPolygon(4, color);
		frontBottom->SetVertex(0, CVertex({ x2, y1, z2 }));
		frontBottom->SetVertex(1, CVertex({ x1, y1, z2 }));
		frontBottom->SetVertex(2, CVertex({ x1, y2, z3 }));
		frontBottom->SetVertex(3, CVertex({ x2, y2, z3 }));
		SetPolygon(i++, frontBottom);

		CPolygon* front = new CPolygon(4, color);
		front->SetVertex(0, CVertex({ x2, y1, z1 }));
		front->SetVertex(1, CVertex({ x1, y1, z1 }));
		front->SetVertex(2, CVertex({ x1, y1, z2 }));
		front->SetVertex(3, CVertex({ x2, y1, z2 }));
		SetPolygon(i++, front);

		CPolygon* backBottom = new CPolygon(4, color);
		backBottom->SetVertex(0, CVertex({ x1, y4, z2 }));
		backBottom->SetVertex(1, CVertex({ x2, y4, z2 }));
		backBottom->SetVertex(2, CVertex({ x2, y3, z3 }));
		backBottom->SetVertex(3, CVertex({ x1, y3, z3 }));
		SetPolygon(i++, backBottom);

		CPolygon* back = new CPolygon(4, color);
		back->SetVertex(0, CVertex({ x1, y4, z1 }));
		back->SetVertex(1, CVertex({ x2, y4, z1 }));
		back->SetVertex(2, CVertex({ x2, y4, z2 }));
		back->SetVertex(3, CVertex({ x1, y4, z2 }));
		SetPolygon(i++, back);
	}

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CTankTurretMesh::CTankTurretMesh() : CMesh(13)
{
	int i = 0;

	{ // 머리
		float x1 = -2.f;	float x2 = 2.f;		float x3 = -2.8f;	float x4 = 2.8f;
		float y1 = 3.f;		float y2 = 2.f;		float y3 = 1.f;		float y4 = -8.f;
		float z1 = -2.f;	float z2 = -1.f;	float z3 = 0.f;

		COLORREF color = RGB(107, 142, 35);

		// 윗면
		CPolygon* top = new CPolygon(4, color);
		top->SetVertex(0, CVertex({ x1, y4, z1 }));
		top->SetVertex(1, CVertex({ x1, y3, z1 }));
		top->SetVertex(2, CVertex({ x2, y3, z1 }));
		top->SetVertex(3, CVertex({ x2, y4, z1 }));
		SetPolygon(i++, top);

		CPolygon* backTop = new CPolygon(4, color);
		backTop->SetVertex(0, CVertex({ x1, y4, z1 }));
		backTop->SetVertex(1, CVertex({ x2, y4, z1 }));
		backTop->SetVertex(2, CVertex({ x4, y4, z2 }));
		backTop->SetVertex(3, CVertex({ x3, y4, z2 }));
		SetPolygon(i++, backTop);

		CPolygon* backBottom = new CPolygon(4, color);
		backBottom->SetVertex(0, CVertex({ x3, y4, z2 }));
		backBottom->SetVertex(1, CVertex({ x4, y4, z2 }));
		backBottom->SetVertex(2, CVertex({ x4, y4, z3 }));
		backBottom->SetVertex(3, CVertex({ x3, y4, z3 }));
		SetPolygon(i++, backBottom);


		CPolygon* rightTop = new CPolygon(4, color);
		rightTop->SetVertex(0, CVertex({ x4, y4, z2 }));
		rightTop->SetVertex(1, CVertex({ x2, y4, z1 }));
		rightTop->SetVertex(2, CVertex({ x2, y3, z1 }));
		rightTop->SetVertex(3, CVertex({ x4, y1, z2 }));
		SetPolygon(i++, rightTop);

		CPolygon* rightBottom = new CPolygon(4, color);
		rightBottom->SetVertex(0, CVertex({ x4, y4, z3 }));
		rightBottom->SetVertex(1, CVertex({ x4, y4, z2 }));
		rightBottom->SetVertex(2, CVertex({ x4, y1, z2 }));
		rightBottom->SetVertex(3, CVertex({ x4, y2, z3 }));
		SetPolygon(i++, rightBottom);

		CPolygon* leftTop = new CPolygon(4, color);
		leftTop->SetVertex(0, CVertex({ x3, y1, z2 }));
		leftTop->SetVertex(1, CVertex({ x1, y3, z1 }));
		leftTop->SetVertex(2, CVertex({ x1, y4, z1 }));
		leftTop->SetVertex(3, CVertex({ x3, y4, z2 }));
		SetPolygon(i++, leftTop);

		CPolygon* leftBottom = new CPolygon(4, color);
		leftBottom->SetVertex(0, CVertex({ x3, y2, z3 }));
		leftBottom->SetVertex(1, CVertex({ x3, y1, z2 }));
		leftBottom->SetVertex(2, CVertex({ x3, y4, z2 }));
		leftBottom->SetVertex(3, CVertex({ x3, y4, z3 }));
		SetPolygon(i++, leftBottom);

		CPolygon* frontTop = new CPolygon(4, color);
		frontTop->SetVertex(0, CVertex({ x4, y1, z2 }));
		frontTop->SetVertex(1, CVertex({ x2, y3, z1 }));
		frontTop->SetVertex(2, CVertex({ x1, y3, z1 }));
		frontTop->SetVertex(3, CVertex({ x3, y1, z2 }));
		SetPolygon(i++, frontTop);

		CPolygon* frontBottom = new CPolygon(4, color);
		frontBottom->SetVertex(0, CVertex({ x4, y2, z3 }));
		frontBottom->SetVertex(1, CVertex({ x4, y1, z2 }));
		frontBottom->SetVertex(2, CVertex({ x3, y1, z2 }));
		frontBottom->SetVertex(3, CVertex({ x3, y2, z3 }));
		SetPolygon(i++, frontBottom);
	}
	{	// 포신
		float x1 = -0.3f;	float x2 = 0.3f;
		float y1 = 20.f;	float y2 = 3.f;		float y3 = 1.f;
		float z1 = -1.8f;	float z2 = -1.2f;

		COLORREF color = RGB(107, 142, 35);

		CPolygon* top = new CPolygon(4, color);
		top->SetVertex(0, CVertex({ x1, y1, z1 }));
		top->SetVertex(1, CVertex({ x2, y1, z1 }));
		top->SetVertex(2, CVertex({ x2, y3, z1 }));
		top->SetVertex(3, CVertex({ x1, y3, z1 }));
		SetPolygon(i++, top);

		CPolygon* right = new CPolygon(4, color);
		right->SetVertex(0, CVertex({ x2, y3, z1 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y1, z2 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);

		CPolygon* left = new CPolygon(4, color);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y1, z1 }));
		left->SetVertex(2, CVertex({ x1, y3, z1 }));
		left->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, left);

		CPolygon* bottom = new CPolygon(4, color);
		bottom->SetVertex(0, CVertex({ x1, y2, z2 }));
		bottom->SetVertex(1, CVertex({ x2, y2, z2 }));
		bottom->SetVertex(2, CVertex({ x2, y1, z2 }));
		bottom->SetVertex(3, CVertex({ x1, y1, z2 }));
		SetPolygon(i++, bottom);
	}

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CEnemyTankMesh::CEnemyTankMesh() : CMesh(32)
{
	int i = 0;


	{ // 몸체
		float x1 = -4.f;	float x2 = 4.f;		float x3 = -5.f;	float x4 = 5.f;
		float y1 = 10.f;	float y2 = 9.f;		float y3 = 6.f;		float y4 = -10.f;
		float z1 = 0.f;		float z2 = 1.5f;		float z3 = 2.f;

		COLORREF color = RGB(128, 0, 0);

		// 윗면
		CPolygon* top = new CPolygon(4, color);
		top->SetVertex(0, CVertex({ x1, y4, z1 }));
		top->SetVertex(1, CVertex({ x1, y3, z1 }));
		top->SetVertex(2, CVertex({ x2, y3, z1 }));
		top->SetVertex(3, CVertex({ x2, y4, z1 }));
		SetPolygon(i++, top);

		CPolygon* backTop = new CPolygon(4, color);
		backTop->SetVertex(0, CVertex({ x1, y4, z1 }));
		backTop->SetVertex(1, CVertex({ x2, y4, z1 }));
		backTop->SetVertex(2, CVertex({ x4, y4, z2 }));
		backTop->SetVertex(3, CVertex({ x3, y4, z2 }));
		SetPolygon(i++, backTop);

		CPolygon* backBottom = new CPolygon(4, color);
		backBottom->SetVertex(0, CVertex({ x3, y4, z2 }));
		backBottom->SetVertex(1, CVertex({ x4, y4, z2 }));
		backBottom->SetVertex(2, CVertex({ x4, y4, z3 }));
		backBottom->SetVertex(3, CVertex({ x3, y4, z3 }));
		SetPolygon(i++, backBottom);


		CPolygon* rightTop = new CPolygon(4, color);
		rightTop->SetVertex(0, CVertex({ x4, y4, z2 }));
		rightTop->SetVertex(1, CVertex({ x2, y4, z1 }));
		rightTop->SetVertex(2, CVertex({ x2, y3, z1 }));
		rightTop->SetVertex(3, CVertex({ x4, y1, z2 }));
		SetPolygon(i++, rightTop);

		CPolygon* rightBottom = new CPolygon(4, color);
		rightBottom->SetVertex(0, CVertex({ x4, y4, z3 }));
		rightBottom->SetVertex(1, CVertex({ x4, y4, z2 }));
		rightBottom->SetVertex(2, CVertex({ x4, y1, z2 }));
		rightBottom->SetVertex(3, CVertex({ x4, y2, z3 }));
		SetPolygon(i++, rightBottom);

		CPolygon* leftTop = new CPolygon(4, color);
		leftTop->SetVertex(0, CVertex({ x3, y1, z2 }));
		leftTop->SetVertex(1, CVertex({ x1, y3, z1 }));
		leftTop->SetVertex(2, CVertex({ x1, y4, z1 }));
		leftTop->SetVertex(3, CVertex({ x3, y4, z2 }));
		SetPolygon(i++, leftTop);

		CPolygon* leftBottom = new CPolygon(4, color);
		leftBottom->SetVertex(0, CVertex({ x3, y2, z3 }));
		leftBottom->SetVertex(1, CVertex({ x3, y1, z2 }));
		leftBottom->SetVertex(2, CVertex({ x3, y4, z2 }));
		leftBottom->SetVertex(3, CVertex({ x3, y4, z3 }));
		SetPolygon(i++, leftBottom);

		CPolygon* frontTop = new CPolygon(4, color);
		frontTop->SetVertex(0, CVertex({ x4, y1, z2 }));
		frontTop->SetVertex(1, CVertex({ x2, y3, z1 }));
		frontTop->SetVertex(2, CVertex({ x1, y3, z1 }));
		frontTop->SetVertex(3, CVertex({ x3, y1, z2 }));
		SetPolygon(i++, frontTop);

		CPolygon* frontBottom = new CPolygon(4, color);
		frontBottom->SetVertex(0, CVertex({ x4, y2, z3 }));
		frontBottom->SetVertex(1, CVertex({ x4, y1, z2 }));
		frontBottom->SetVertex(2, CVertex({ x3, y1, z2 }));
		frontBottom->SetVertex(3, CVertex({ x3, y2, z3 }));
		SetPolygon(i++, frontBottom);
	}
	{ // 오른쪽 바퀴
		float x1 = 3.f;		float x2 = 4.8f;
		float y1 = 8.8f;	float y2 = 8.f;		float y3 = -9.f;		float y4 = -9.8f;
		float z1 = 2.f;		float z2 = 2.5f;	float z3 = 3.f;

		COLORREF color = RGB(0, 0, 0);

		CPolygon* bottom = new CPolygon(4, color);
		bottom->SetVertex(0, CVertex({ x2, y2, z3 }));
		bottom->SetVertex(1, CVertex({ x1, y2, z3 }));
		bottom->SetVertex(2, CVertex({ x1, y3, z3 }));
		bottom->SetVertex(3, CVertex({ x2, y3, z3 }));
		SetPolygon(i++, bottom);

		CPolygon* frontBottom = new CPolygon(4, color);
		frontBottom->SetVertex(0, CVertex({ x2, y1, z2 }));
		frontBottom->SetVertex(1, CVertex({ x1, y1, z2 }));
		frontBottom->SetVertex(2, CVertex({ x1, y2, z3 }));
		frontBottom->SetVertex(3, CVertex({ x2, y2, z3 }));
		SetPolygon(i++, frontBottom);

		CPolygon* front = new CPolygon(4, color);
		front->SetVertex(0, CVertex({ x2, y1, z1 }));
		front->SetVertex(1, CVertex({ x1, y1, z1 }));
		front->SetVertex(2, CVertex({ x1, y1, z2 }));
		front->SetVertex(3, CVertex({ x2, y1, z2 }));
		SetPolygon(i++, front);

		CPolygon* backBottom = new CPolygon(4, color);
		backBottom->SetVertex(0, CVertex({ x1, y4, z2 }));
		backBottom->SetVertex(1, CVertex({ x2, y4, z2 }));
		backBottom->SetVertex(2, CVertex({ x2, y3, z3 }));
		backBottom->SetVertex(3, CVertex({ x1, y3, z3 }));
		SetPolygon(i++, backBottom);

		CPolygon* back = new CPolygon(4, color);
		back->SetVertex(0, CVertex({ x1, y4, z1 }));
		back->SetVertex(1, CVertex({ x2, y4, z1 }));
		back->SetVertex(2, CVertex({ x2, y4, z2 }));
		back->SetVertex(3, CVertex({ x1, y4, z2 }));
		SetPolygon(i++, back);
	}
	{ // 왼쪽 바퀴
		float x1 = -4.8f;		float x2 = -3.f;
		float y1 = 8.8f;	float y2 = 8.f;		float y3 = -9.f;		float y4 = -9.8f;
		float z1 = 2.f;		float z2 = 2.5f;	float z3 = 3.f;

		COLORREF color = RGB(0, 0, 0);

		CPolygon* bottom = new CPolygon(4, color);
		bottom->SetVertex(0, CVertex({ x2, y2, z3 }));
		bottom->SetVertex(1, CVertex({ x1, y2, z3 }));
		bottom->SetVertex(2, CVertex({ x1, y3, z3 }));
		bottom->SetVertex(3, CVertex({ x2, y3, z3 }));
		SetPolygon(i++, bottom);

		CPolygon* frontBottom = new CPolygon(4, color);
		frontBottom->SetVertex(0, CVertex({ x2, y1, z2 }));
		frontBottom->SetVertex(1, CVertex({ x1, y1, z2 }));
		frontBottom->SetVertex(2, CVertex({ x1, y2, z3 }));
		frontBottom->SetVertex(3, CVertex({ x2, y2, z3 }));
		SetPolygon(i++, frontBottom);

		CPolygon* front = new CPolygon(4, color);
		front->SetVertex(0, CVertex({ x2, y1, z1 }));
		front->SetVertex(1, CVertex({ x1, y1, z1 }));
		front->SetVertex(2, CVertex({ x1, y1, z2 }));
		front->SetVertex(3, CVertex({ x2, y1, z2 }));
		SetPolygon(i++, front);

		CPolygon* backBottom = new CPolygon(4, color);
		backBottom->SetVertex(0, CVertex({ x1, y4, z2 }));
		backBottom->SetVertex(1, CVertex({ x2, y4, z2 }));
		backBottom->SetVertex(2, CVertex({ x2, y3, z3 }));
		backBottom->SetVertex(3, CVertex({ x1, y3, z3 }));
		SetPolygon(i++, backBottom);

		CPolygon* back = new CPolygon(4, color);
		back->SetVertex(0, CVertex({ x1, y4, z1 }));
		back->SetVertex(1, CVertex({ x2, y4, z1 }));
		back->SetVertex(2, CVertex({ x2, y4, z2 }));
		back->SetVertex(3, CVertex({ x1, y4, z2 }));
		SetPolygon(i++, back);
	}
	{ // 머리
		float x1 = -2.f;	float x2 = 2.f;		float x3 = -2.8f;	float x4 = 2.8f;
		float y1 = 3.f;		float y2 = 2.f;		float y3 = 1.f;		float y4 = -8.f;
		float z1 = -2.f;	float z2 = -1.f;	float z3 = 0.f;

		COLORREF color = RGB(128, 0, 0);

		// 윗면
		CPolygon* top = new CPolygon(4, color);
		top->SetVertex(0, CVertex({ x1, y4, z1 }));
		top->SetVertex(1, CVertex({ x1, y3, z1 }));
		top->SetVertex(2, CVertex({ x2, y3, z1 }));
		top->SetVertex(3, CVertex({ x2, y4, z1 }));
		SetPolygon(i++, top);

		CPolygon* backTop = new CPolygon(4, color);
		backTop->SetVertex(0, CVertex({ x1, y4, z1 }));
		backTop->SetVertex(1, CVertex({ x2, y4, z1 }));
		backTop->SetVertex(2, CVertex({ x4, y4, z2 }));
		backTop->SetVertex(3, CVertex({ x3, y4, z2 }));
		SetPolygon(i++, backTop);

		CPolygon* backBottom = new CPolygon(4, color);
		backBottom->SetVertex(0, CVertex({ x3, y4, z2 }));
		backBottom->SetVertex(1, CVertex({ x4, y4, z2 }));
		backBottom->SetVertex(2, CVertex({ x4, y4, z3 }));
		backBottom->SetVertex(3, CVertex({ x3, y4, z3 }));
		SetPolygon(i++, backBottom);


		CPolygon* rightTop = new CPolygon(4, color);
		rightTop->SetVertex(0, CVertex({ x4, y4, z2 }));
		rightTop->SetVertex(1, CVertex({ x2, y4, z1 }));
		rightTop->SetVertex(2, CVertex({ x2, y3, z1 }));
		rightTop->SetVertex(3, CVertex({ x4, y1, z2 }));
		SetPolygon(i++, rightTop);

		CPolygon* rightBottom = new CPolygon(4, color);
		rightBottom->SetVertex(0, CVertex({ x4, y4, z3 }));
		rightBottom->SetVertex(1, CVertex({ x4, y4, z2 }));
		rightBottom->SetVertex(2, CVertex({ x4, y1, z2 }));
		rightBottom->SetVertex(3, CVertex({ x4, y2, z3 }));
		SetPolygon(i++, rightBottom);

		CPolygon* leftTop = new CPolygon(4, color);
		leftTop->SetVertex(0, CVertex({ x3, y1, z2 }));
		leftTop->SetVertex(1, CVertex({ x1, y3, z1 }));
		leftTop->SetVertex(2, CVertex({ x1, y4, z1 }));
		leftTop->SetVertex(3, CVertex({ x3, y4, z2 }));
		SetPolygon(i++, leftTop);

		CPolygon* leftBottom = new CPolygon(4, color);
		leftBottom->SetVertex(0, CVertex({ x3, y2, z3 }));
		leftBottom->SetVertex(1, CVertex({ x3, y1, z2 }));
		leftBottom->SetVertex(2, CVertex({ x3, y4, z2 }));
		leftBottom->SetVertex(3, CVertex({ x3, y4, z3 }));
		SetPolygon(i++, leftBottom);

		CPolygon* frontTop = new CPolygon(4, color);
		frontTop->SetVertex(0, CVertex({ x4, y1, z2 }));
		frontTop->SetVertex(1, CVertex({ x2, y3, z1 }));
		frontTop->SetVertex(2, CVertex({ x1, y3, z1 }));
		frontTop->SetVertex(3, CVertex({ x3, y1, z2 }));
		SetPolygon(i++, frontTop);

		CPolygon* frontBottom = new CPolygon(4, color);
		frontBottom->SetVertex(0, CVertex({ x4, y2, z3 }));
		frontBottom->SetVertex(1, CVertex({ x4, y1, z2 }));
		frontBottom->SetVertex(2, CVertex({ x3, y1, z2 }));
		frontBottom->SetVertex(3, CVertex({ x3, y2, z3 }));
		SetPolygon(i++, frontBottom);
	}
	{	// 포신
		float x1 = -0.3f;	float x2 = 0.3f;
		float y1 = 20.f;	float y2 = 3.f;		float y3 = 1.f;
		float z1 = -1.8f;	float z2 = -1.2f;

		COLORREF color = RGB(128, 0, 0);

		CPolygon* top = new CPolygon(4, color);
		top->SetVertex(0, CVertex({ x1, y1, z1 }));
		top->SetVertex(1, CVertex({ x2, y1, z1 }));
		top->SetVertex(2, CVertex({ x2, y3, z1 }));
		top->SetVertex(3, CVertex({ x1, y3, z1 }));
		SetPolygon(i++, top);

		CPolygon* right = new CPolygon(4, color);
		right->SetVertex(0, CVertex({ x2, y3, z1 }));
		right->SetVertex(1, CVertex({ x2, y1, z1 }));
		right->SetVertex(2, CVertex({ x2, y1, z2 }));
		right->SetVertex(3, CVertex({ x2, y2, z2 }));
		SetPolygon(i++, right);

		CPolygon* left = new CPolygon(4, color);
		left->SetVertex(0, CVertex({ x1, y1, z2 }));
		left->SetVertex(1, CVertex({ x1, y1, z1 }));
		left->SetVertex(2, CVertex({ x1, y3, z1 }));
		left->SetVertex(3, CVertex({ x1, y2, z2 }));
		SetPolygon(i++, left);

		CPolygon* bottom = new CPolygon(4, color);
		bottom->SetVertex(0, CVertex({ x1, y2, z2 }));
		bottom->SetVertex(1, CVertex({ x2, y2, z2 }));
		bottom->SetVertex(2, CVertex({ x2, y1, z2 }));
		bottom->SetVertex(3, CVertex({ x1, y1, z2 }));
		SetPolygon(i++, bottom);
	}

	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(5.f, 10.f, 2.f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}