#pragma once
#include "KamataEngine.h"

class MapChipField;

class ArrowBullet
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const KamataEngine::Vector3& velocity);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ワールド座標取得
	KamataEngine::Vector3 GetWorldPosition() const;

	// 消滅しているか
	bool IsDead() const
	{
		return isDead_;
	}

	// 強制的に消滅
	void SetDead()
	{
		isDead_ = true; 
	}

	void SetMapChipField(MapChipField* mapChipField)
	{ 
		mapChipField_ = mapChipField;
	}

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Camera* camera_ = nullptr;

	// 移動速度
	KamataEngine::Vector3 velocity_ = {};

	// 生存時間
	float lifeTimer_ = 0.0f;

	// 最大生存時間
	static inline const float kLifeTime = 5.0f;

	// 消滅しているか
	bool isDead_ = false;

	MapChipField* mapChipField_ = nullptr;

	KamataEngine::Vector3 startPosition_;

};