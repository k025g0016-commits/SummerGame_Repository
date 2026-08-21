#pragma once
#include "KamataEngine.h"

class Player;

class CameraController
{
public:
	// カメラの移動可能範囲
	struct Rect 
	{
		float left = 0.0f;
		float right = 0.0f;
		float bottom = 0.0f;
		float top = 0.0f;
	};

public:
	// 初期化
	void Initialize(KamataEngine::Camera* camera, const Player* player);

	// 更新
	void Update();

	// カメラの移動可能範囲を設定
	void SetMovableArea(const Rect& movableArea) 
	{
		movableArea_ = movableArea;
	}

	// ボスエリア用の移動範囲を設定
	void SetBossArea(float left, float right);

private:
	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// 追従対象のプレイヤー
	const Player* player_ = nullptr;

	// プレイヤーから見たカメラの位置
	KamataEngine::Vector3 targetOffset_ =
	{
		0.0f,
		2.0f,
		-15.0f
	};

	// カメラの移動可能範囲
	Rect movableArea_ =
	{
	    0.0f,  // left
	    30.0f, // right
	    0.0f,  // bottom
	    10.0f  // top
	};

	// 追従の補間率
	static inline const float kInterpolationRate = 0.1f;

	// カメラから見える横幅の半分
	static inline const float kCameraViewHalfWidth = 8.0f;

};