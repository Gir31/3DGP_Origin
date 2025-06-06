#include "Mesh.h"

CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}
CMesh::~CMesh()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	if (m_pd3dIndexBuffer) m_pd3dIndexBuffer->Release();
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	if (m_pVertices) delete[] m_pVertices;
	if (m_pnIndices) delete[] m_pnIndices;
}

void CMesh::ReleaseUploadBuffers()
{
	//정점 버퍼를 위한 업로드 버퍼를 소멸시킨다.
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	m_pd3dVertexUploadBuffer = NULL;
	if (m_pd3dIndexUploadBuffer) m_pd3dIndexUploadBuffer->Release();
	m_pd3dIndexUploadBuffer = NULL;
}


void CMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
	if (m_pd3dIndexBuffer)
	{
		pd3dCommandList->IASetIndexBuffer(&m_d3dIndexBufferView);
		pd3dCommandList->DrawIndexedInstanced(m_nIndices, 1, 0, 0, 0);
		//인덱스 버퍼가 있으면 인덱스 버퍼를 파이프라인(IA: 입력 조립기)에 연결하고 인덱스를 사용하여 렌더링한다.
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

int CMesh::CheckRayIntersection(XMFLOAT3& xmf3RayOrigin, XMFLOAT3& xmf3RayDirection,
	float* pfNearHitDistance)
{
	//하나의 메쉬에서 광선은 여러 개의 삼각형과 교차할 수 있다. 교차하는 삼각형들 중 가장 가까운 삼각형을 찾는다.
	int nIntersections = 0;
	BYTE* pbPositions = (BYTE*)m_pVertices;
	int nOffset = (m_d3dPrimitiveTopology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? 3 : 1;
	/*메쉬의 프리미티브(삼각형)들의 개수이다. 삼각형 리스트인 경우 (정점의 개수 / 3) 또는 (인덱스의 개수 / 3), 삼각
   형 스트립의 경우 (정점의 개수 - 2) 또는 (인덱스의 개수 ? 2)이다.*/
	int nPrimitives = (m_d3dPrimitiveTopology == D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ?
		(m_nVertices / 3) : (m_nVertices - 2);
	if (m_nIndices > 0) nPrimitives = (m_d3dPrimitiveTopology ==
		D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST) ? (m_nIndices / 3) : (m_nIndices - 2);
	//광선은 모델 좌표계로 표현된다.
	XMVECTOR xmRayOrigin = XMLoadFloat3(&xmf3RayOrigin);
	XMVECTOR xmRayDirection = XMLoadFloat3(&xmf3RayDirection);
	//모델 좌표계의 광선과 메쉬의 바운딩 박스(모델 좌표계)와의 교차를 검사한다.
	bool bIntersected = m_xmBoundingBox.Intersects(xmRayOrigin, xmRayDirection,
		*pfNearHitDistance);
	//모델 좌표계의 광선이 메쉬의 바운딩 박스와 교차하면 메쉬와의 교차를 검사한다.
	if (bIntersected)
	{
		float fNearHitDistance = FLT_MAX;
		/*메쉬의 모든 프리미티브(삼각형)들에 대하여 픽킹 광선과의 충돌을 검사한다. 충돌하는 모든 삼각형을 찾아 광선의
	   시작점(실제로는 카메라 좌표계의 원점)에 가장 가까운 삼각형을 찾는다.*/
		for (int i = 0; i < nPrimitives; i++)
		{
			XMVECTOR v0 = XMLoadFloat3((XMFLOAT3*)(pbPositions + ((m_pnIndices) ?
				(m_pnIndices[(i * nOffset) + 0]) : ((i * nOffset) + 0)) * m_nStride));
			XMVECTOR v1 = XMLoadFloat3((XMFLOAT3*)(pbPositions + ((m_pnIndices) ?
				(m_pnIndices[(i * nOffset) + 1]) : ((i * nOffset) + 1)) * m_nStride));
			XMVECTOR v2 = XMLoadFloat3((XMFLOAT3*)(pbPositions + ((m_pnIndices) ?
				(m_pnIndices[(i * nOffset) + 2]) : ((i * nOffset) + 2)) * m_nStride));
			float fHitDistance;
			BOOL bIntersected = TriangleTests::Intersects(xmRayOrigin, xmRayDirection, v0,
				v1, v2, fHitDistance);
			if (bIntersected)
			{
				if (fHitDistance < fNearHitDistance)
				{
					*pfNearHitDistance = fNearHitDistance = fHitDistance;
				}
				nIntersections++;
			}
		}
	}
	return(nIntersections);
}

CTriangleMesh::CTriangleMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
	//삼각형 메쉬를 정의한다.
	m_nVertices = 3;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	/*정점(삼각형의 꼭지점)의 색상은 시계방향 순서대로 빨간색, 녹색, 파란색으로 지정한다. RGBA(Red, Green, Blue,
   Alpha) 4개의 파라메터를 사용하여 색상을 표현한다. 각 파라메터는 0.0~1.0 사이의 실수값을 가진다.*/
	CDiffusedVertex pVertices[3];
	pVertices[0] = CDiffusedVertex(XMFLOAT3(0.0f, 0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f,
		1.0f));
	pVertices[1] = CDiffusedVertex(XMFLOAT3(0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f,
		1.0f));
	pVertices[2] = CDiffusedVertex(XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(Colors::Blue));
	//삼각형 메쉬를 리소스(정점 버퍼)로 생성한다.
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	//정점 버퍼 뷰를 생성한다.
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}



CCubeMeshDiffused::CCubeMeshDiffused(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList
	* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice,
		pd3dCommandList)
{
	//직육면체는 꼭지점(정점)이 8개이다.
	m_nVertices = 8;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	//정점 버퍼는 직육면체의 꼭지점 8개에 대한 정점 데이터를 가진다.
	CDiffusedVertex pVertices[8];
	pVertices[0] = CDiffusedVertex(XMFLOAT3(-fx, +fy, -fz), RANDOM_COLOR);
	pVertices[1] = CDiffusedVertex(XMFLOAT3(+fx, +fy, -fz), RANDOM_COLOR);
	pVertices[2] = CDiffusedVertex(XMFLOAT3(+fx, +fy, +fz), RANDOM_COLOR);
	pVertices[3] = CDiffusedVertex(XMFLOAT3(-fx, +fy, +fz), RANDOM_COLOR);
	pVertices[4] = CDiffusedVertex(XMFLOAT3(-fx, -fy, -fz), RANDOM_COLOR);
	pVertices[5] = CDiffusedVertex(XMFLOAT3(+fx, -fy, -fz), RANDOM_COLOR);
	pVertices[6] = CDiffusedVertex(XMFLOAT3(+fx, -fy, +fz), RANDOM_COLOR);
	pVertices[7] = CDiffusedVertex(XMFLOAT3(-fx, -fy, +fz), RANDOM_COLOR);
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
	/*인덱스 버퍼는 직육면체의 6개의 면(사각형)에 대한 기하 정보를 갖는다. 삼각형 리스트로 직육면체를 표현할 것이
   므로 각 면은 2개의 삼각형을 가지고 각 삼각형은 3개의 정점이 필요하다. 즉, 인덱스 버퍼는 전체 36(=6*2*3)개의 인
   덱스를 가져야 한다.*/
	m_nIndices = 36;
	UINT pnIndices[36];
	// 앞면(Front) 사각형의 위쪽 삼각형
	pnIndices[0] = 3; pnIndices[1] = 1; pnIndices[2] = 0;
	// 앞면(Front) 사각형의 아래쪽 삼각형
	pnIndices[3] = 2; pnIndices[4] = 1; pnIndices[5] = 3;
	// 윗면(Top) 사각형의 위쪽 삼각형
	pnIndices[6] = 0; pnIndices[7] = 5; pnIndices[8] = 4;
	// 윗면(Top) 사각형의 아래쪽 삼각형
	pnIndices[9] = 1; pnIndices[10] = 5; pnIndices[11] = 0;
	// 뒷면(Back) 사각형의 위쪽 삼각형
	pnIndices[12] = 3; pnIndices[13] = 4; pnIndices[14] = 7;
	// 뒷면(Back) 사각형의 아래쪽 삼각형
	pnIndices[15] = 0; pnIndices[16] = 4; pnIndices[17] = 3;
	// 아래면(Bottom) 사각형의 위쪽 삼각형
	pnIndices[18] = 1; pnIndices[19] = 6; pnIndices[20] = 5;
	// 아래면(Bottom) 사각형의 아래쪽 삼각형
	pnIndices[21] = 2; pnIndices[22] = 6; pnIndices[23] = 1;
	// 옆면(Left) 사각형의 위쪽 삼각형
	pnIndices[24] = 2; pnIndices[25] = 7; pnIndices[26] = 6;
	// 옆면(Left) 사각형의 아래쪽 삼각형
	pnIndices[27] = 3; pnIndices[28] = 7; pnIndices[29] = 2;
	// 옆면(Right) 사각형의 위쪽 삼각형
	pnIndices[30] = 6; pnIndices[31] = 4; pnIndices[32] = 5;
	// 옆면(Right) 사각형의 아래쪽 삼각형
	pnIndices[33] = 7; pnIndices[34] = 4; pnIndices[35] = 6;
	//인덱스 버퍼를 생성한다.
	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pnIndices,
		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_pd3dIndexUploadBuffer);
	//인덱스 버퍼 뷰를 생성한다.
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;
}
CCubeMeshDiffused::~CCubeMeshDiffused()
{
}

//따라하기12
CAirplaneMeshDiffused::CAirplaneMeshDiffused(ID3D12Device* pd3dDevice,
	ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth,
	XMFLOAT4 xmf4Color) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 24 * 3;
	m_nStride = sizeof(CDiffusedVertex);
	m_nOffset = 0;
	m_nSlot = 0;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	//위의 그림과 같은 비행기 메쉬를 표현하기 위한 정점 데이터이다.
	CDiffusedVertex pVertices[24 * 3];
	float x1 = fx * 0.2f, y1 = fy * 0.2f, x2 = fx * 0.1f, y3 = fy * 0.3f, y2 = ((y1 - (fy -
		y3)) / x1) * x2 + (fy - y3);
	int i = 0;
	//비행기 메쉬의 위쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 아래쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 오른쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 뒤쪽/오른쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(+x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 왼쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), -fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, +(fy + y3), +fz),
		Vector4::Add(xmf4Color, RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x2, +y2, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	//비행기 메쉬의 뒤쪽/왼쪽 면
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(0.0f, 0.0f, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-x1, -y1, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, +fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	pVertices[i++] = CDiffusedVertex(XMFLOAT3(-fx, -y3, -fz), Vector4::Add(xmf4Color,
		RANDOM_COLOR));
	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}
CAirplaneMeshDiffused::~CAirplaneMeshDiffused()
{

}

CCartMesh::CCartMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CMesh(pd3dDevice, pd3dCommandList)
{

	XMFLOAT4 bodyColor = XMFLOAT4(0, 100, 0, 1);
	XMFLOAT4 innerColor = XMFLOAT4(166, 73, 53, 1);
	XMFLOAT4 seatColor = XMFLOAT4(211, 178, 129, 1);
	XMFLOAT4 barColor = XMFLOAT4(50, 50, 50, 1);
	XMFLOAT4 frameColor = XMFLOAT4(100, 100, 100, 1);

	int v = 0;
	int idx = 0;

	m_nVertices = 266;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_pVertices = new CDiffusedVertex[m_nVertices];

	m_nIndices = 420;
	m_pnIndices = new UINT[m_nIndices];

	{
		// 좌표 설정
		float x1 = -4.0f, x2 = 4.0f;
		float y1 = 10.0f, y2 = -10.0f;
		float z1 = 3.0f, z2 = 2.0f;
		XMFLOAT4 color = XMFLOAT4(0, 0, 0, 1);

		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), color); // 좌상하
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), color); // 우상하
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), color); // 우하하
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), color); // 좌하하

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color); // 좌상상
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color); // 우상상
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), color); // 우하상
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), color); // 좌하상

		// 앞면(Front)
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면(Back)
		m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 3;
		m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 1;

		// 윗면(Top)
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 0;

		// 아랫면(Bottom)
		m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3;

		// 왼쪽(Left)
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 7;
		m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 오른쪽(Right)
		m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 1;

		v += 8;
	}

	{
		float x1 = -3.8f, x2 = 3.8f;
		float y1 = -8.8f, y2 = -9.8f; // 앞(y1) 뒤(y2)
		float z1 = 2.0f, z2 = -2.0f;  // 위(z1) 아래(z2)

		// 윗면 (Top) z = z2
		m_pVertices[v] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), frameColor); // 0
		m_pVertices[v+1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), frameColor); // 1
		m_pVertices[v+2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), frameColor); // 2
		m_pVertices[v+3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), frameColor); // 3

		// 앞면 (Front) y = y1
		m_pVertices[v+4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), innerColor); // 4
		m_pVertices[v+5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), innerColor); // 5
		m_pVertices[v+6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), innerColor); // 6
		m_pVertices[v+7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), innerColor); // 7

		// 뒷면 (Back) y = y2
		m_pVertices[v+8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 8
		m_pVertices[v+9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 9
		m_pVertices[v+10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 10
		m_pVertices[v+11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 11

		// 왼쪽면 (Left) x = x1
		m_pVertices[v+12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), bodyColor); // 12
		m_pVertices[v+13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 13
		m_pVertices[v+14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 14
		m_pVertices[v+15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 15

		// 오른쪽면 (Right) x = x2
		m_pVertices[v+16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), bodyColor); // 16
		m_pVertices[v+17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 17
		m_pVertices[v+18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 18
		m_pVertices[v+19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 19

		// 윗면
		m_pnIndices[idx++] = v; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v;

		// 앞면
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽면
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽면
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		v += 20;
	}

	{
		float x1 = 2.8f, x2 = 3.8f;
		float y1 = 9.8f, y2 = -9.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f, z2 = 0.0f;   // 위(z1), 아래(z2)

		// 윗면 (Top) z = z2 
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), frameColor); // 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), frameColor); // 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), frameColor); // 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), frameColor); // 3

		// 앞면 (Front) y = y1
		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), bodyColor); // 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), bodyColor); // 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 7

		// 뒷면 (Back) y = y2
		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 11

		// 왼쪽 (Left) x = x1
		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), innerColor); // 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), innerColor); // 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), innerColor); // 14
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), innerColor); // 15

		// 오른쪽 (Right) x = x2
		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), bodyColor); // 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 17
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 18
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 19

		// 인덱스 설정 

		// 윗면
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽면
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽면
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// 정점 인덱스 증가
		v += 20;

	}
	// Cart 왼쪽 벽				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = -2.8f;
		float y1 = 9.8f;	float y2 = -9.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = 0.0f;  // 아래위 (Z축)

		// 윗면 (Top) z = z2
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), frameColor); // 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), frameColor); // 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), frameColor); // 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), frameColor); // 3

		// 앞면 (Front) y = y1
		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), bodyColor); // 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), bodyColor); // 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 7

		// 뒷면 (Back) y = y2
		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 11

		// 왼쪽 (Left) x = x1
		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), bodyColor); // 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 14
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 15

		// 오른쪽 (Right) x = x2
		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), innerColor); // 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), innerColor); // 17
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), innerColor); // 18
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), innerColor); // 19

		// 인덱스 설정

		// 윗면
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽면
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽면
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// 정점 인덱스 증가
		v += 20;

	}
	// Cart 앞쪽 벽				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = 9.8f;	float y2 = 8.8f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = 0.0f;  // 아래위 (Z축)

		// 윗면 (Top)
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), frameColor); // 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), frameColor); // 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), frameColor); // 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), frameColor); // 3

		// 앞면 (Front) y = y1
		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), bodyColor); // 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), bodyColor); // 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 7

		// 뒷면 (Back) y = y2
		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 11

		// 왼쪽 (Left) x = x1
		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), bodyColor); // 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 14
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 15

		// 오른쪽 (Right) x = x2
		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), bodyColor); // 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 17
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 18
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 19

		// 윗면
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽면
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽면
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		v += 20;
	}
	// Cart 본넷				| 사용 Mesh : 5
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = 9.8f;	float y2 = 3.8f;  // 앞뒤 (Y축)
		float z1 = 0.0f;	float z2 = -1.0f;  // 아래위 (Z축)

		// 아래면 (Bottom)
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 3

		// 뒷면 (Back)
		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 7

		// 윗면 (Top)
		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 11

		// 오른쪽 (Right) – 삼각형
		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), bodyColor); // 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), bodyColor); // 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), bodyColor); // 14

		// 왼쪽 (Left) – 삼각형
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), bodyColor); // 15
		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), bodyColor); // 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), bodyColor); // 17

		// 아래면 (Bottom)
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 뒷면 (Back)
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 윗면 (Top)
		m_pnIndices[idx++] = v + 8;  m_pnIndices[idx++] = v + 9;  m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 오른쪽 (Right) – 단일 삼각형
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;

		// 왼쪽 (Left) – 단일 삼각형
		m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17;

		v += 18; // 총 정점 18개 사용
	}
	// 앞쪽 좌석
	{
		float x1 = -2.8f;	float x2 = 2.8f;
		float y1 = -2.0f;	float y2 = -3.0f;  // 앞뒤 (Y축)
		float z1 = 2.0f;	float z2 = -1.0f;  // 아래위 (Z축)

		// 시트 정점 설정 (총 20개)
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(-2.8f, -2.0f, -1.0f), seatColor); // 윗면 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(2.8f, -2.0f, -1.0f), seatColor); // 윗면 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(2.8f, -3.0f, -1.0f), seatColor); // 윗면 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(-2.8f, -3.0f, -1.0f), seatColor); // 윗면 3

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(2.8f, -2.0f, -1.0f), seatColor); // 앞면 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(-2.8f, -2.0f, -1.0f), seatColor); // 앞면 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(-2.8f, -2.0f, 2.0f), seatColor); // 앞면 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(2.8f, -2.0f, 2.0f), seatColor); // 앞면 7

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(-2.8f, -3.0f, -1.0f), seatColor); // 뒷면 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(2.8f, -3.0f, -1.0f), seatColor); // 뒷면 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(2.8f, -3.0f, 2.0f), seatColor); // 뒷면 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(-2.8f, -3.0f, 2.0f), seatColor); // 뒷면 11

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(-2.8f, -2.0f, -1.0f), seatColor); // 왼쪽 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(-2.8f, -3.0f, -1.0f), seatColor); // 왼쪽 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(-2.8f, -3.0f, 2.0f), seatColor); // 왼쪽 14
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(-2.8f, -2.0f, 2.0f), seatColor); // 왼쪽 15

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(2.8f, -2.0f, -1.0f), seatColor); // 오른쪽 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(2.8f, -2.0f, 2.0f), seatColor); // 오른쪽 17
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(2.8f, -3.0f, 2.0f), seatColor); // 오른쪽 18
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(2.8f, -3.0f, -1.0f), seatColor); // 오른쪽 19

		// 시트 인덱스 설정 (총 30개 = 삼각형 10개)
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0; // 윗면

		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4; // 앞면

		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8; // 뒷면

		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12; // 왼쪽

		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16; // 오른쪽

		v += 20;
	}
	// 앞쪽 좌석 머리 1
	{
		float x1 = -1.9f;	float x2 = -0.9f;
		float y1 = -2.2f;	float y2 = -2.8f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 시트 정점 생성
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // 윗면 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // 윗면 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor); // 윗면 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // 윗면 3

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // 앞면 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // 앞면 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor); // 앞면 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor); // 앞면 7

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // 뒷면 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor); // 뒷면 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor); // 뒷면 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor); // 뒷면 11

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // 왼쪽 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // 왼쪽 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor); // 왼쪽 14
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor); // 왼쪽 15

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // 오른쪽 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor); // 오른쪽 17
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor); // 오른쪽 18
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor); // 오른쪽 19

		// 시트 인덱스 생성
		// 윗면
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		v += 20; // 정점 증가

	}
	// 앞쪽 좌석 머리 2
	{
		float x1 = 0.9f;	float x2 = 1.9f;
		float y1 = -2.2f;	float y2 = -2.8f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 정점 정의
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // top 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // top 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor); // top 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // top 3

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // front 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // front 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor); // front 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor); // front 7

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // back 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor); // back 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor); // back 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor); // back 11

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // left 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // left 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor); // left 14
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor); // left 15

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // right 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor); // right 17
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor); // right 18
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor); // right 19

		// 인덱스 정의
		// 윗면
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면
		m_pnIndices[idx++] = v + 8;  m_pnIndices[idx++] = v + 9;  m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		v += 20; // 정점 증가
	}
	// 앞쪽 안전 바
	{
		// 좌표 및 색상 정의
		float x1 = -3.8f, x2 = 3.8f;
		float y1 = 1.0f, y2 = 0.5f;   // 앞(y1), 뒤(y2)
		float z1 = 0.0f, z2 = -0.5f;  // 아래(z1), 위(z2)

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), barColor); // top 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), barColor); // top 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), barColor); // top 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), barColor); // top 3

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), barColor); // front 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), barColor); // front 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), barColor); // front 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), barColor); // front 7

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), barColor); // back 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), barColor); // back 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), barColor); // back 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), barColor); // back 11

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), barColor); // left 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), barColor); // left 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), barColor); // left 14
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), barColor); // left 15

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), barColor); // right 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), barColor); // right 17
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), barColor); // right 18
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), barColor); // right 19

		// 인덱스 설정
		// 윗면
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면
		m_pnIndices[idx++] = v + 8;  m_pnIndices[idx++] = v + 9;  m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// 정점 인덱스 증가
		v += 20;

	}
	// 뒤쪽 좌석
	{
		float x1 = -2.8f, x2 = 2.8f;
		float y1 = -7.8f, y2 = -8.8f;  // 앞(y1), 뒤(y2)
		float z1 = 2.0f, z2 = -1.0f;  // 위(z1), 아래(z2)

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // top
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // front
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor);
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // back
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor);
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor);

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // left
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor);

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // right
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);

		// 인덱스 설정
		// 윗면 (Top)
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면 (Front)
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면 (Back)
		m_pnIndices[idx++] = v + 8;  m_pnIndices[idx++] = v + 9;  m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽 (Left)
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽 (Right)
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// 정점 인덱스 증가
		v += 20;

	}
	// 뒤쪽 좌석 머리 1
	{
		float x1 = -1.9f;	float x2 = -0.9f;
		float y1 = -8.0f;	float y2 = -8.6f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // top
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // front
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor);
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // back
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor);
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor);

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // left
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor);

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // right
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);

		// 인덱스 설정
		// 윗면 (Top)
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면 (Front)
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면 (Back)
		m_pnIndices[idx++] = v + 8;  m_pnIndices[idx++] = v + 9;  m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽 (Left)
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽 (Right)
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// 정점 인덱스 증가
		v += 20;
	}
	// 뒤쪽 좌석 머리 2
	{
		float x1 = 0.9f;	float x2 = 1.9f;
		float y1 = -8.0f;	float y2 = -8.6f;  // 앞뒤 (Y축)
		float z1 = -1.0f;	float z2 = -1.75f;  // 아래위 (Z축)

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // top
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // front
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor);
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor); // back
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor);
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor);

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), seatColor); // left
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), seatColor);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), seatColor);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), seatColor);

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), seatColor); // right
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), seatColor);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), seatColor);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), seatColor);

		// 인덱스 설정
		// 윗면 (Top)
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면 (Front)
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면 (Back)
		m_pnIndices[idx++] = v + 8;  m_pnIndices[idx++] = v + 9;  m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽 (Left)
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽 (Right)
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// 정점 인덱스 증가
		v += 20;
	}
	// 뒤쪽 안전 바
	{
		float x1 = -3.8f;	float x2 = 3.8f;
		float y1 = -4.8f;	float y2 = -5.3f;  // 앞뒤 (Y축)
		float z1 = 0.0f;	float z2 = -0.5f;  // 아래위 (Z축)

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), barColor); // top 0
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), barColor); // top 1
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), barColor); // top 2
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), barColor); // top 3

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), barColor); // front 4
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), barColor); // front 5
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), barColor); // front 6
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), barColor); // front 7

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), barColor); // back 8
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), barColor); // back 9
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), barColor); // back 10
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), barColor); // back 11

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), barColor); // left 12
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), barColor); // left 13
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x1, y2, z1), barColor); // left 14
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), barColor); // left 15

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), barColor); // right 16
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), barColor); // right 17
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y2, z1), barColor); // right 18
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), barColor); // right 19

		// 인덱스 설정
		// 윗면
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// 앞면
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// 뒷면
		m_pnIndices[idx++] = v + 8;  m_pnIndices[idx++] = v + 9;  m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// 왼쪽
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// 오른쪽
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// 정점 인덱스 증가
		v += 20;
	}

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnIndices,
		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_pd3dIndexUploadBuffer);
	//인덱스 버퍼 뷰를 생성한다.
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

	// [마무리] 바운딩 박스 설정
	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

XMFLOAT3 CatmullRom(const XMFLOAT3& p0, const XMFLOAT3& p1, const XMFLOAT3& p2, const XMFLOAT3& p3, float t)
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

CRailMesh::CRailMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	float width, float height, float depth) : CMesh(pd3dDevice, pd3dCommandList)
{
	std::array<XMFLOAT3, 41> points = {
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

	float railSamplingStep = 0.05f;
	float railWidth = 1.f;
	float railHeight = 1.f;
	float deltaT = 0.01f;

	XMFLOAT4 color = XMFLOAT4(150.f/255.f, 150.f / 255.f, 150.f / 255.f, 1.f);

	m_nVertices = 8*(points.size() - 3)*20;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_pVertices = new CDiffusedVertex[m_nVertices];

	m_nIndices = 36 * (points.size() - 3) * 20;
	m_pnIndices = new UINT[m_nIndices];

	int v = 0; int idx = 0;

	for (size_t i = 0; i + 3 < points.size(); ++i)
	{
		const XMFLOAT3& p0 = points[i];
		const XMFLOAT3& p1 = points[i + 1];
		const XMFLOAT3& p2 = points[i + 2];
		const XMFLOAT3& p3 = points[i + 3];

		for (float t = 0.0f; t <= 1.0f; t += railSamplingStep)
		{
			XMFLOAT3 pos = CatmullRom(p0, p1, p2, p3, t);
			float tAhead = (t + deltaT > 1.0f) ? 1.0f : (t + deltaT);
			XMFLOAT3 posAhead = CatmullRom(p0, p1, p2, p3, tAhead);

			m_pVertices[v] = CDiffusedVertex(XMFLOAT3(pos.x - railWidth, pos.y + railHeight, pos.z), color); // 0
			m_pVertices[v+1] = CDiffusedVertex(XMFLOAT3(pos.x + railWidth, pos.y + railHeight, pos.z), color); // 1
			m_pVertices[v+2] = CDiffusedVertex(XMFLOAT3(posAhead.x + railWidth, posAhead.y + railHeight, posAhead.z), color); // 3
			m_pVertices[v+3] = CDiffusedVertex(XMFLOAT3(posAhead.x - railWidth, posAhead.y + railHeight, posAhead.z), color); // 2
			m_pVertices[v+4] = CDiffusedVertex(XMFLOAT3(pos.x - railWidth, pos.y - railHeight, pos.z), color); // 1
			m_pVertices[v+5] = CDiffusedVertex(XMFLOAT3(pos.x + railWidth, pos.y - railHeight, pos.z), color); // 0
			m_pVertices[v+6] = CDiffusedVertex(XMFLOAT3(posAhead.x + railWidth, posAhead.y - railHeight, posAhead.z), color); // 2
			m_pVertices[v+7] = CDiffusedVertex(XMFLOAT3(posAhead.x - railWidth, posAhead.y - railHeight, posAhead.z), color); // 3

			m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 0;
			// 앞면(Front) 사각형의 아래쪽 삼각형
			m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 3;
			// 윗면(Top) 사각형의 위쪽 삼각형
			m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 4;
			// 윗면(Top) 사각형의 아래쪽 삼각형
			m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 0;
			// 뒷면(Back) 사각형의 위쪽 삼각형
			m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 7;
			// 뒷면(Back) 사각형의 아래쪽 삼각형
			m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 3;
			// 아래면(Bottom) 사각형의 위쪽 삼각형
			m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 5;
			// 아래면(Bottom) 사각형의 아래쪽 삼각형
			m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 1;
			// 옆면(Left) 사각형의 위쪽 삼각형
			m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 6;
			// 옆면(Left) 사각형의 아래쪽 삼각형
			m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 2;
			// 옆면(Right) 사각형의 위쪽 삼각형
			m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5;
			// 옆면(Right) 사각형의 아래쪽 삼각형
			m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 6;

			v += 8;
		}
	}

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnIndices,
		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_pd3dIndexUploadBuffer);
	//인덱스 버퍼 뷰를 생성한다.
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CTankBodyMesh::CTankBodyMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CMesh(pd3dDevice, pd3dCommandList)
{
	int v = 0;
	int idx = 0;

	m_nVertices = 62;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_pVertices = new CDiffusedVertex[m_nVertices];

	m_nIndices = 114;
	m_pnIndices = new UINT[m_nIndices];

	{ // 몸체
		float x1 = -4.f, x2 = 4.f, x3 = -5.f, x4 = 5.f;
		float y1 = 10.f, y2 = 9.f, y3 = 6.f, y4 = -10.f;
		float z1 = 0.f, z2 = 1.5f, z3 = 2.f;

		XMFLOAT4 color = XMFLOAT4(107 / 255.f, 142 / 255.f, 35 / 255.f, 1.f); // RGB 정규화

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color); // top
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x4, y4, z2), color); // backTop
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x3, y4, z2), color);

		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x4, y4, z3), color); // backBottom
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x3, y4, z3), color);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x4, y1, z2), color); // rightTop
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x4, y2, z3), color); // rightBottom

		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x3, y1, z2), color); // leftTop
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x3, y2, z3), color); // leftBottom

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color); // shared
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);

		// frontTop
		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x4, y1, z2), color);
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x3, y1, z2), color);

		// frontBottom
		m_pVertices[v + 20] = CDiffusedVertex(XMFLOAT3(x4, y2, z3), color);
		m_pVertices[v + 21] = CDiffusedVertex(XMFLOAT3(x3, y2, z3), color);

		// 인덱스 설정
		// top
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// backTop
		m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 3;

		// backBottom
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 5;

		// rightTop
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 4;

		// rightBottom
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 8;
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 6;

		// leftTop
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 0;
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 10;

		// leftBottom
		m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 11;

		// frontTop
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// frontBottom
		m_pnIndices[idx++] = v + 20; m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 19;
		m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 21; m_pnIndices[idx++] = v + 20;

		v += 22; // 사용된 정점 수
	}
	{ // 오른쪽 바퀴
		float x1 = 3.f, x2 = 4.8f;
		float y1 = 8.8f, y2 = 8.f, y3 = -9.f, y4 = -9.8f;
		float z1 = 2.f, z2 = 2.5f, z3 = 3.f;

		XMFLOAT4 color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); // 검은색

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x2, y2, z3), color); // bottom
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x1, y2, z3), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x1, y3, z3), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x2, y3, z3), color);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color); // frontBottom
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color);
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y2, z3), color);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y2, z3), color);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), color); // front
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), color);
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color);
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color);

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y4, z2), color); // backBottom
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x2, y4, z2), color);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x2, y3, z3), color);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y3, z3), color);

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color); // back
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y4, z2), color);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x1, y4, z2), color);

		// 인덱스 설정
		// bottom
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// frontBottom
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// front
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// backBottom
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// back
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		v += 20;
	}
	{ // 왼쪽 바퀴
		float x1 = -4.8f, x2 = -3.f;
		float y1 = 8.8f, y2 = 8.f, y3 = -9.f, y4 = -9.8f;
		float z1 = 2.f, z2 = 2.5f, z3 = 3.f;

		XMFLOAT4 color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); // 검은색

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x2, y2, z3), color); // bottom
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x1, y2, z3), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x1, y3, z3), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x2, y3, z3), color);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color); // frontBottom
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color);
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y2, z3), color);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y2, z3), color);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), color); // front
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), color);
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color);
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color);

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y4, z2), color); // backBottom
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x2, y4, z2), color);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x2, y3, z3), color);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y3, z3), color);

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color); // back
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y4, z2), color);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x1, y4, z2), color);

		// 인덱스 설정
		// bottom
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// frontBottom
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// front
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// backBottom
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// back
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		v += 20;
	}

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnIndices,
		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_pd3dIndexUploadBuffer);
	//인덱스 버퍼 뷰를 생성한다.
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

	// [마무리] 바운딩 박스 설정
	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));

}

CTankTurretMesh::CTankTurretMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CMesh(pd3dDevice, pd3dCommandList)
{
	int v = 0;
	int idx = 0;

	m_nVertices = 24;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_pVertices = new CDiffusedVertex[m_nVertices];

	m_nIndices = 78;
	m_pnIndices = new UINT[m_nIndices];

	{
		float x1 = -2.f, x2 = 2.f, x3 = -2.8f, x4 = 2.8f;
		float y1 = 3.f, y2 = 2.f, y3 = 1.f, y4 = -8.f;
		float z1 = -2.f, z2 = -1.f, z3 = 0.f;

		XMFLOAT4 color = XMFLOAT4(100 / 255.f, 150 / 255.f, 50 / 255.f, 1.f); // RGB 정규화

		// 정점 설정
		// top
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color);
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);

		// backTop
		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x4, y4, z2), color);
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x3, y4, z2), color);

		// backBottom
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x4, y4, z3), color);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x3, y4, z3), color);

		// rightTop
		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x4, y1, z2), color);

		// rightBottom
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x4, y2, z3), color);

		// leftTop
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x3, y1, z2), color);

		// leftBottom
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x3, y2, z3), color);

		// frontTop
		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);

		// frontBottom
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x3, y1, z2), color);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x4, y2, z3), color);

		// 인덱스 설정
		// top
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// backTop
		m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 3;

		// backBottom
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 5;

		// rightTop
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 4;

		// rightBottom
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 8;
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 6;

		// leftTop
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 0;
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 10;

		// leftBottom
		m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 11;

		// frontTop
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13;
		m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 8;

		// frontBottom
		m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 15;

		v += 16;
	}

	{	// 포신 (Cannon Barrel)
		float x1 = -0.3f, x2 = 0.3f;
		float y1 = 20.f, y2 = 3.f, y3 = 1.f;
		float z1 = -1.8f, z2 = -1.2f;

		XMFLOAT4 color = XMFLOAT4(100 / 255.f, 150 / 255.f, 50 / 255.f, 1.f); // RGB 정규화

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), color); // top
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color); // right
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), color);

		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color); // left
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), color);

		// 인덱스 설정
		// top face
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// right face
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 4;
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 2;

		// left face
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 3;
		m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 6;

		// bottom face
		m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 4;
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7;

		v += 8; // 총 정점 수
	}


	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnIndices,
		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_pd3dIndexUploadBuffer);
	//인덱스 버퍼 뷰를 생성한다.
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;

	// [마무리] 바운딩 박스 설정
	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0, 0, 0), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}

CEnemyTankMesh::CEnemyTankMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
	: CMesh(pd3dDevice, pd3dCommandList)
{
	int v = 0;
	int idx = 0;

	m_nVertices = 86;
	m_nStride = sizeof(CDiffusedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	m_pVertices = new CDiffusedVertex[m_nVertices];

	m_nIndices = 192;
	m_pnIndices = new UINT[m_nIndices];

	{ // 몸체
		float x1 = -4.f, x2 = 4.f, x3 = -5.f, x4 = 5.f;
		float y1 = 10.f, y2 = 9.f, y3 = 6.f, y4 = -10.f;
		float z1 = 0.f, z2 = 1.5f, z3 = 2.f;

		XMFLOAT4 color = XMFLOAT4(128 / 255.f, 0.f, 0.f, 1.f);

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color); // top
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x4, y4, z2), color); // backTop
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x3, y4, z2), color);

		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x4, y4, z3), color); // backBottom
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x3, y4, z3), color);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x4, y1, z2), color); // rightTop
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x4, y2, z3), color); // rightBottom

		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x3, y1, z2), color); // leftTop
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x3, y2, z3), color); // leftBottom

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color); // shared
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);

		// frontTop
		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x4, y1, z2), color);
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x3, y1, z2), color);

		// frontBottom
		m_pVertices[v + 20] = CDiffusedVertex(XMFLOAT3(x4, y2, z3), color);
		m_pVertices[v + 21] = CDiffusedVertex(XMFLOAT3(x3, y2, z3), color);

		// 인덱스 설정
		// top
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// backTop
		m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 3;

		// backBottom
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 5;

		// rightTop
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 4;

		// rightBottom
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 8;
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 6;

		// leftTop
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 0;
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 10;

		// leftBottom
		m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 11;

		// frontTop
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		// frontBottom
		m_pnIndices[idx++] = v + 20; m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 19;
		m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 21; m_pnIndices[idx++] = v + 20;

		v += 22; // 사용된 정점 수
	}
	{ // 오른쪽 바퀴
		float x1 = 3.f, x2 = 4.8f;
		float y1 = 8.8f, y2 = 8.f, y3 = -9.f, y4 = -9.8f;
		float z1 = 2.f, z2 = 2.5f, z3 = 3.f;

		XMFLOAT4 color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); // 검은색

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x2, y2, z3), color); // bottom
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x1, y2, z3), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x1, y3, z3), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x2, y3, z3), color);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color); // frontBottom
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color);
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y2, z3), color);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y2, z3), color);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), color); // front
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), color);
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color);
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color);

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y4, z2), color); // backBottom
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x2, y4, z2), color);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x2, y3, z3), color);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y3, z3), color);

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color); // back
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y4, z2), color);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x1, y4, z2), color);

		// 인덱스 설정
		// bottom
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// frontBottom
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// front
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// backBottom
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// back
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		v += 20;
	}
	{ // 왼쪽 바퀴
		float x1 = -4.8f, x2 = -3.f;
		float y1 = 8.8f, y2 = 8.f, y3 = -9.f, y4 = -9.8f;
		float z1 = 2.f, z2 = 2.5f, z3 = 3.f;

		XMFLOAT4 color = XMFLOAT4(0.f, 0.f, 0.f, 1.f); // 검은색

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x2, y2, z3), color); // bottom
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x1, y2, z3), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x1, y3, z3), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x2, y3, z3), color);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color); // frontBottom
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color);
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y2, z3), color);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x2, y2, z3), color);

		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), color); // front
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), color);
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color);
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color);

		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x1, y4, z2), color); // backBottom
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x2, y4, z2), color);
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x2, y3, z3), color);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x1, y3, z3), color);

		m_pVertices[v + 16] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color); // back
		m_pVertices[v + 17] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);
		m_pVertices[v + 18] = CDiffusedVertex(XMFLOAT3(x2, y4, z2), color);
		m_pVertices[v + 19] = CDiffusedVertex(XMFLOAT3(x1, y4, z2), color);

		// 인덱스 설정
		// bottom
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// frontBottom
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 4;

		// front
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 8;

		// backBottom
		m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 14;
		m_pnIndices[idx++] = v + 14; m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 12;

		// back
		m_pnIndices[idx++] = v + 16; m_pnIndices[idx++] = v + 17; m_pnIndices[idx++] = v + 18;
		m_pnIndices[idx++] = v + 18; m_pnIndices[idx++] = v + 19; m_pnIndices[idx++] = v + 16;

		v += 20;
	}
	{
		float x1 = -2.f, x2 = 2.f, x3 = -2.8f, x4 = 2.8f;
		float y1 = 3.f, y2 = 2.f, y3 = 1.f, y4 = -8.f;
		float z1 = -2.f, z2 = -1.f, z3 = 0.f;

		XMFLOAT4 color = XMFLOAT4(200 / 255.f, 0.f, 0.f, 1.f);

		// 정점 설정
		// top
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y4, z1), color);
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x2, y4, z1), color);

		// backTop
		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x4, y4, z2), color);
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x3, y4, z2), color);

		// backBottom
		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x4, y4, z3), color);
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x3, y4, z3), color);

		// rightTop
		m_pVertices[v + 8] = CDiffusedVertex(XMFLOAT3(x4, y1, z2), color);

		// rightBottom
		m_pVertices[v + 9] = CDiffusedVertex(XMFLOAT3(x4, y2, z3), color);

		// leftTop
		m_pVertices[v + 10] = CDiffusedVertex(XMFLOAT3(x3, y1, z2), color);

		// leftBottom
		m_pVertices[v + 11] = CDiffusedVertex(XMFLOAT3(x3, y2, z3), color);

		// frontTop
		m_pVertices[v + 12] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 13] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);

		// frontBottom
		m_pVertices[v + 14] = CDiffusedVertex(XMFLOAT3(x3, y1, z2), color);
		m_pVertices[v + 15] = CDiffusedVertex(XMFLOAT3(x4, y2, z3), color);

		// 인덱스 설정
		// top
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// backTop
		m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 3;

		// backBottom
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 6;
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 5;

		// rightTop
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 4;

		// rightBottom
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 8;
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 9; m_pnIndices[idx++] = v + 6;

		// leftTop
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 0;
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 10;

		// leftBottom
		m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 5;
		m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 11;

		// frontTop
		m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 12; m_pnIndices[idx++] = v + 13;
		m_pnIndices[idx++] = v + 13; m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 8;

		// frontBottom
		m_pnIndices[idx++] = v + 15; m_pnIndices[idx++] = v + 8; m_pnIndices[idx++] = v + 10;
		m_pnIndices[idx++] = v + 10; m_pnIndices[idx++] = v + 11; m_pnIndices[idx++] = v + 15;

		v += 16;
	}
	{	// 포신 (Cannon Barrel)
		float x1 = -0.3f, x2 = 0.3f;
		float y1 = 20.f, y2 = 3.f, y3 = 1.f;
		float z1 = -1.8f, z2 = -1.2f;

		XMFLOAT4 color = XMFLOAT4(200 / 255.f, 0.f, 0.f, 1.f);

		// 정점 설정
		m_pVertices[v + 0] = CDiffusedVertex(XMFLOAT3(x1, y1, z1), color); // top
		m_pVertices[v + 1] = CDiffusedVertex(XMFLOAT3(x2, y1, z1), color);
		m_pVertices[v + 2] = CDiffusedVertex(XMFLOAT3(x2, y3, z1), color);
		m_pVertices[v + 3] = CDiffusedVertex(XMFLOAT3(x1, y3, z1), color);

		m_pVertices[v + 4] = CDiffusedVertex(XMFLOAT3(x2, y1, z2), color); // right
		m_pVertices[v + 5] = CDiffusedVertex(XMFLOAT3(x2, y2, z2), color);

		m_pVertices[v + 6] = CDiffusedVertex(XMFLOAT3(x1, y1, z2), color); // left
		m_pVertices[v + 7] = CDiffusedVertex(XMFLOAT3(x1, y2, z2), color);

		// 인덱스 설정
		// top face
		m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 2;
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 0;

		// right face
		m_pnIndices[idx++] = v + 2; m_pnIndices[idx++] = v + 1; m_pnIndices[idx++] = v + 4;
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 2;

		// left face
		m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 0; m_pnIndices[idx++] = v + 3;
		m_pnIndices[idx++] = v + 3; m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 6;

		// bottom face
		m_pnIndices[idx++] = v + 7; m_pnIndices[idx++] = v + 5; m_pnIndices[idx++] = v + 4;
		m_pnIndices[idx++] = v + 4; m_pnIndices[idx++] = v + 6; m_pnIndices[idx++] = v + 7;

		v += 8; // 총 정점 수
	}

	m_pd3dVertexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pVertices,
		m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT,
		D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);
	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;

	m_pd3dIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pnIndices,
		sizeof(UINT) * m_nIndices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER,
		&m_pd3dIndexUploadBuffer);
	//인덱스 버퍼 뷰를 생성한다.
	m_d3dIndexBufferView.BufferLocation = m_pd3dIndexBuffer->GetGPUVirtualAddress();
	m_d3dIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	m_d3dIndexBufferView.SizeInBytes = sizeof(UINT) * m_nIndices;


	m_xmBoundingBox = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(5.f, 10.f, 2.f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
}