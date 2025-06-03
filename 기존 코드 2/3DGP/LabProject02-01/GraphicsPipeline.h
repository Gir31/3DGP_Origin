#pragma once

#include "GameObject.h"
#include "Camera.h"

class CGraphicsPipeline
{
private:
	static XMFLOAT4X4*		m_pxmf4x4World;
	static XMFLOAT4X4*		m_pxmf4x4ViewProject;
	static CViewport*		m_pViewport;

public:
	static void SetWorldTransform(XMFLOAT4X4* pxmf4x4World) { m_pxmf4x4World = pxmf4x4World; }
	static void SetViewPerspectiveProjectTransform(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4ViewPerspectiveProject);
	static void SetViewOrthographicProjectTransform(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4OrthographicProject);
	static void SetViewport(ID3D12GraphicsCommandList* pd3dCommandList, D3D12_VIEWPORT* pViewport) { pd3dCommandList->RSSetViewports(1, pViewport); }


	static XMFLOAT3 ScreenTransform(XMFLOAT3& xmf3Project);
	static XMFLOAT3 Project(XMFLOAT3& xmf3Model);
	static XMFLOAT3 Transform(XMFLOAT3& xmf3Model);
};
