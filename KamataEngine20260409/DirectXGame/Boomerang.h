#pragma once
#include "KamataEngine.h"

class Player;
class MapChipField;

class Boomerang 
{
public:
	enum class Phase 
	{
		kHeld,     // プレイヤーが持っている
		kOutbound, // 行き
		kReturn    // 帰り
	};

public:
	// 初期化
	void Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera, Player* player);

	// 更新
	void Update();

	// 描画
	void Draw();

	// 投げる
	void Throw();

	// 現在投げられているか
	bool IsThrown() const 
	{ 
		return phase_ != Phase::kHeld; 
	}

	// 現在の状態を取得
	Phase GetPhase() const
	{
		return phase_;
	}

	// ワールド座標取得
	KamataEngine::Vector3 GetWorldPosition() const;

	void StartReturn();

	// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField)
	{
		mapChipField_ = mapChipField; 
	}

private:
	// 所持中
	void UpdateHeld();

	// 行き
	void UpdateOutbound();

	// 帰り
	void UpdateReturn();

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	const KamataEngine::Camera* camera_ = nullptr;

	Player* player_ = nullptr;

	Phase phase_ = Phase::kHeld;

	KamataEngine::Vector3 velocity_ = {};

	KamataEngine::Vector3 throwStartPosition_ = {};

	static inline const float kThrowSpeed = 0.25f;
	static inline const float kReturnSpeed = 0.3f;
	static inline const float kMaxDistance = 7.0f;
	static inline const float kCatchDistance = 0.4f;
	static inline const float kRotationSpeed = 0.2f;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	// 行きのブーメランが地形に当たったか
	bool CheckMapCollision() const;

};