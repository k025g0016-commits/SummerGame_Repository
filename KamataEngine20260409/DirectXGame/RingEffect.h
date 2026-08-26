#pragma once
#include "KamataEngine.h"

class RingEffect 
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 終了したか
	bool IsFinished() const 
	{
		return isFinished_; 
	}

private:
	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// ワールド変換
	KamataEngine::WorldTransform worldTransform_;

	// 経過時間
	float timer_ = 0.0f;

	// 終了したか
	bool isFinished_ = false;

	// 表示時間
	static inline const float kDuration = 0.2f;

	// 初期サイズ
	static inline const float kStartScale = 0.5f;

	// 最終サイズ
	static inline const float kEndScale = 1.5f;
};
