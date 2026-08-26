#pragma once
#include "KamataEngine.h"

class SparkParticle 
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

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

	// 移動速度
	KamataEngine::Vector3 velocity_{};

	// 経過時間
	float timer_ = 0.0f;

	// 終了したか
	bool isFinished_ = false;

	// 生存時間
	static inline const float kLifeTime = 0.25f;

	// 初期サイズ
	static inline const float kStartScale = 0.5f;

	// 終了時サイズ
	static inline const float kEndScale = 0.0f;
};
