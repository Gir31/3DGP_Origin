#include "stdafx.h"
#include "GraphicsPipeline.h"

XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4World = NULL;
XMFLOAT4X4* CGraphicsPipeline::m_pxmf4x4ViewProject = NULL;
CViewport* CGraphicsPipeline::m_pViewport = NULL;

void CGraphicsPipeline::SetViewPerspectiveProjectTransform(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4VP)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, pxmf4x4VP, 0); // 16 * float4 = 64byte
}

void CGraphicsPipeline::SetViewOrthographicProjectTransform(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4VO)
{
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, pxmf4x4VO, 0);
}

XMFLOAT3 CGraphicsPipeline::Transform(XMFLOAT3& xmf3Model)
{
	XMFLOAT3 xmf3Project = Project(xmf3Model);
	XMFLOAT3 f3Screen = ScreenTransform(xmf3Project);

	return(f3Screen);
}

XMFLOAT3 CGraphicsPipeline::Project(XMFLOAT3& xmf3Model)
{
	XMFLOAT4X4 xmf4x4Transform = Matrix4x4::Multiply(*m_pxmf4x4World, *m_pxmf4x4ViewProject);
	XMFLOAT3 xmf3Project = Vector3::TransformCoord(xmf3Model, xmf4x4Transform);

	return(xmf3Project);
}

XMFLOAT3 CGraphicsPipeline::ScreenTransform(XMFLOAT3& xmf3Project)
{
	XMFLOAT3 f3Screen = xmf3Project;

	float fHalfWidth = m_pViewport->m_nWidth * 0.5f;
	float fHalfHeight = m_pViewport->m_nHeight * 0.5f;
	f3Screen.x = m_pViewport->m_nLeft + (xmf3Project.x * fHalfWidth) + fHalfWidth;
	f3Screen.y = m_pViewport->m_nTop + (-xmf3Project.y * fHalfHeight) + fHalfHeight;

	return(f3Screen);
}
