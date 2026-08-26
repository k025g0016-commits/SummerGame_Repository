#pragma once
#include "KamataEngine.h"
#include <cstdint>

class ShutterDoor
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, int32_t groupId);

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

	// グループ番号を取得
	int32_t GetGroupId() const 
	{ 
		return groupId_;
	}

private:
	KamataEngine::WorldTransform worldTransform_;
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	bool isOpen_ = false;

	KamataEngine::Vector3 startPosition_ = {};

	float openTargetY_ = 0.0f;

	static inline const float kOpenSpeed = 0.05f;
	static inline const float kOpenDistance = 3.0f;

	bool isHidden_ = false;

	// グループ番号
	int32_t groupId_ = 0;
};