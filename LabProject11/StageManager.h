#pragma once
#include "Scene.h"

class StageManager
{
private:
	std::array<CScene*, 4> stageArr{};
	std::array<CPlayer*, 4> playerArr{};

	int currLevel = 0;
	int nextLevel = 0;

	bool ready = false;
	bool change = false;

	float elapsedTime = 0.f;

public:
	StageManager(CScene* stage1, CScene* stage2, CScene* stage3, CScene* stage4);
	~StageManager();

	void setReady(bool rflag);
	void setChange(bool cflag);
	void setNextLevel(int level);
	void setPlayer(CPlayer* player, int stage);
	bool getReady();
	bool getChange();
	int getCurrLevel();
	CPlayer* getPlayer(int stage);

	void buildStage(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void releaseStage();

	void changeStage(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12CommandQueue* pd3dCommandQueue, ID3D12CommandAllocator* pd3dCommandAllocator,
		CPlayer*& pPlayer, CCamera*& pCamera, int nextStage);
	void waitTime(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12CommandQueue* pd3dCommandQueue, ID3D12CommandAllocator* pd3dCommandAllocator,
		CPlayer*& pPlayer, CCamera*& pCamera, float fElapsedTime);

	CScene* getCurrStage();

	ID3D12Device* sm_pd3dDevice = nullptr;
	ID3D12GraphicsCommandList* sm_pd3dCommandList = nullptr;
};

