#pragma once
#include "KamataEngine.h"
#include <array>

class DeathParticle 
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

	// パーティクル数
	static inline const uint32_t kNumParticles = 8;

	// 8個分のワールド変換
	std::array<KamataEngine::WorldTransform, kNumParticles> worldTransforms_;

	// 経過時間
	float timer_ = 0.0f;

	// 終了したか
	bool isFinished_ = false;

	// 表示時間
	static inline const float kDuration = 0.6f;

	// 移動速度
	static inline const float kSpeed = 0.08f;

	// 1個あたりの角度
	static inline const float kAngleUnit = 2.0f * 3.1415926535f / static_cast<float>(kNumParticles);

	// 色
	KamataEngine::ObjectColor objectColor_;

	KamataEngine::Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
};
