#pragma once
#include "KamataEngine.h"
#include <numbers>

class GameOverScene
{
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	~GameOverScene();

	// タイトルへ戻る要求が出ているか
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

	// 前フレームの入力
	bool wasSpaceKeyPressed_ = false;

	// SPACE案内の点滅用
	float spaceBlinkTimer_ = 0.0f;
	bool isSpaceWordVisible_ = true;

	// 点滅間隔
	static inline const float kSpaceBlinkInterval = 0.5f;

	// ゲームオーバーBGM
	uint32_t gameOverBGMHandle_ = 0;

	// ゲームオーバーBGM再生ハンドル
	uint32_t gameOverBGMVoiceHandle_ = 0;

	// ゲームオーバーBGMを開始したか
	bool isGameOverBGMStarted_ = false;

};