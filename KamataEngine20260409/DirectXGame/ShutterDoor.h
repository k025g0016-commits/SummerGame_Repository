#pragma once
#include "KamataEngine.h"

class ShutterDoor
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition() const;

		// ドアを開く
	void Open();

	// 開いているか
	bool IsOpen() const
	{ 
		return isOpen_;
	}

	// 完全に開いて非表示になっているか
	bool IsHidden() const 
	{
		return isHidden_;
	}

private:
	// ワールド変換
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

		// 開いているか
	bool isOpen_ = false;

	// 初期位置
	KamataEngine::Vector3 startPosition_ = {};

	// 開いたときの目標Y座標
	float openTargetY_ = 0.0f;

	// 開く速度
	static inline const float kOpenSpeed = 0.05f;

	// 上へ移動する距離
	static inline const float kOpenDistance = 3.0f;

	// 開ききって消えているか
	bool isHidden_ = false;

};