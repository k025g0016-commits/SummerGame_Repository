#pragma once
#include "KamataEngine.h"
#include <numbers>

class GameClearScene 
{
public:
	void Initialize();
	void Update();
	void Draw();

	~GameClearScene();

	bool IsReturnTitleRequested() const 
	{
		return returnTitleRequested_; 
	}

private:
	// モデル
	KamataEngine::Model* backgroundModel_ = nullptr;
	KamataEngine::Model* wordModel_ = nullptr;
	KamataEngine::Model* spaceWordModel_ = nullptr;

	// ワールド変換
	KamataEngine::WorldTransform backgroundTransform_;
	KamataEngine::WorldTransform wordTransform_;
	KamataEngine::WorldTransform spaceWordTransform_;

	// カメラ
	KamataEngine::Camera camera_;

	// タイトルへ戻る要求
	bool returnTitleRequested_ = false;

	// 前フレームでSpaceキーが押されていたか
	bool wasSpaceKeyPressed_ = false;

	// SPACE案内の点滅
	float spaceBlinkTimer_ = 0.0f;
	bool isSpaceWordVisible_ = true;

	static inline const float kSpaceBlinkInterval = 0.5f;
};