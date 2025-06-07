#include "stdafx.h"
#include "Scene.h"
#include "Player.h"
#include "StageManager.h"

StageManager::StageManager(CScene* stage1, CScene* stage2, CScene* stage3, CScene* stage4)
{
	stageArr[0] = stage1;
	stageArr[1] = stage2;
	stageArr[2] = stage3;
	stageArr[3] = stage4;
}


void StageManager::setReady(bool rflag)
{
	ready = rflag;
}
void StageManager::setChange(bool cflag)
{
	change = cflag;
}
void StageManager::setNextLevel(int level)
{
	nextLevel = level;
}
void StageManager::setPlayer(CPlayer* player, int stage)
{
	playerArr[stage] = player;
}
bool StageManager::getReady()
{
	return ready;
}
bool StageManager::getChange()
{
	return change;
}
int StageManager::getCurrLevel()
{
	return currLevel;
}

CPlayer* StageManager::getPlayer(int stage)
{
	return playerArr[stage];
}

void StageManager::buildStage(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	stageArr[currLevel]->BuildObjects(pd3dDevice, pd3dCommandList);
}

void StageManager::releaseStage()
{
	stageArr[currLevel]->ReleaseObjects();
}

void StageManager::changeStage(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	ID3D12CommandQueue* pd3dCommandQueue, ID3D12CommandAllocator* pd3dCommandAllocator, 
	CPlayer* pPlayer, CCamera* pCamera, int level)
{
	if (change) {
		stageArr[currLevel]->ReleaseObjects();
		currLevel = level;

		pd3dCommandAllocator->Reset();
		pd3dCommandList->Reset(pd3dCommandAllocator, nullptr);

		stageArr[currLevel]->BuildObjects(pd3dDevice, pd3dCommandList);

		switch (currLevel) {
		case 0: {
			CAirplanePlayer* pAirplanePlayer = new CAirplanePlayer(pd3dDevice,
				pd3dCommandList, getCurrStage()->GetGraphicsRootSignature());
			pPlayer = pAirplanePlayer;
			pCamera = pPlayer->GetCamera();
			break;
		}

		case 1:
		{
			CAirplanePlayer * pAirplanePlayer = new CAirplanePlayer(pd3dDevice,
				pd3dCommandList, getCurrStage()->GetGraphicsRootSignature());
			pPlayer = pAirplanePlayer;
			pCamera = pPlayer->GetCamera();
			break;
		}
		case 2:
		{
			CCartPlayer* pCartPlayer = new CCartPlayer(pd3dDevice,
				pd3dCommandList, getCurrStage()->GetGraphicsRootSignature());
			pPlayer = pCartPlayer;
			pCamera = pPlayer->GetCamera();
			break;
		}
		case 3:
		{
			CTank* pTankPlayer = new CTank(pd3dDevice,
				pd3dCommandList, getCurrStage()->GetGraphicsRootSignature());
			pPlayer = pTankPlayer;
			pCamera = pPlayer->GetCamera();
			break;
		}
		}

		pd3dCommandList->Close();

		ID3D12CommandList* ppd3dCommandLists[] = { pd3dCommandList };
		pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	}
}

void StageManager::waitTime(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, 
	ID3D12CommandQueue* pd3dCommandQueue, ID3D12CommandAllocator* pd3dCommandAllocator, 
	CPlayer* pPlayer, CCamera* pCamera, float fElapsedTime)
{
	elapsedTime += fElapsedTime;

	if (elapsedTime > 5.f) {
		elapsedTime = 0.f;
		ready = false;
		change = true;

		changeStage(pd3dDevice, pd3dCommandList, pd3dCommandQueue, pd3dCommandAllocator, pPlayer, pCamera, nextLevel);
	}
}

CScene* StageManager::getCurrStage()
{
	return stageArr[currLevel];
}
