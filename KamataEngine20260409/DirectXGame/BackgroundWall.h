#pragma once
#include "KamataEngine.h"

class BackGroundWall
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera);

	// 更新
	void Update();

	// 描画
	void Draw();

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	const KamataEngine::Camera* camera_ = nullptr;
};