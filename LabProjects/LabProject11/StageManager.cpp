#include "stdafx.h"
#include "Scene.h"
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

void StageManager::changeStage(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, int level)
{
	if (change) {
		stageArr[currLevel]->ReleaseObjects();
		currLevel = level;

		stageArr[currLevel]->BuildObjects(pd3dDevice, pd3dCommandList);
		change = false;
	}
}

void StageManager::waitTime(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fElapsedTime)
{
	elapsedTime += fElapsedTime;

	if (elapsedTime > 5.f) {
		elapsedTime = 0.f;
		ready = false;
		change = true;

		changeStage(pd3dDevice, pd3dCommandList, nextLevel);
	}
}

CScene* StageManager::getCurrStage()
{
	return stageArr[currLevel];
}