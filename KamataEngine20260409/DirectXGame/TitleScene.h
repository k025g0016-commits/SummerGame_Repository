#pragma once
#include "KamataEngine.h"
#include <numbers>

class TitleScene
{
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// タイトルBGM停止
	void StopBGM();

	// タイトルBGMのフェード更新
	void UpdateBGMFade();

	// デストラクタ
	~TitleScene();

	// ゲーム開始要求が出ているか
	bool IsStartRequested() const
	{ 
		return startRequested_;
	}

private:
	// モデル
	KamataEngine::Model* backgroundModel_ = nullptr;
	KamataEngine::Model* titleWordModel_ = nullptr;
	KamataEngine::Model* pressSpaceModel_ = nullptr;

	// プレイヤー・盾モデル
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* shieldModel_ = nullptr;

	// プレイヤー・盾のワールド変換
	KamataEngine::WorldTransform playerTransform_;
	KamataEngine::WorldTransform shieldTransform_;

	// ワールド変換
	KamataEngine::WorldTransform backgroundTransform_;
	KamataEngine::WorldTransform titleWordTransform_;
	KamataEngine::WorldTransform pressSpaceTransform_;

	// カメラ
	KamataEngine::Camera camera_;

	// ゲーム開始要求
	bool startRequested_ = false;

	// 前フレームでSpaceキーが押されていたか
	bool wasSpaceKeyPressed_ = false;

	// SPACE案内の点滅
	float spaceBlinkTimer_ = 0.0f;
	bool isSpaceWordVisible_ = true;

	static inline const float kSpaceBlinkInterval = 0.5f;

	// タイトル画面の盾の回転角
	float shieldRotationAngle_ = 0.0f;

	// 盾が回る速さ
	static inline const float kShieldRotationSpeed = 0.01f;

	float playerFloatTimer_ = 0.0f;
	float playerBaseY_ = -0.5f;

	static inline const float kPlayerFloatSpeed = 1.5f;
	static inline const float kPlayerFloatRange = 0.15f;

	// タイトルBGM
	uint32_t titleBGMHandle_ = 0;

	// タイトルBGM再生ハンドル
	uint32_t titleBGMVoiceHandle_ = 0;

	// タイトルBGMを開始したか
	bool isTitleBGMStarted_ = false;

	// タイトルBGMフェードアウト中か
	bool isTitleBGMFadingOut_ = false;

	// タイトルBGMフェード時間
	float titleBGMFadeTimer_ = 0.0f;

	// タイトルBGM通常音量
	static inline const float kTitleBGMVolume = 0.5f;

	// タイトルBGMフェード時間
	static inline const float kTitleBGMFadeDuration = 0.5f;

};