#pragma once
#include "KamataEngine.h"
#include <array>

class MapChipField;
class Player;

// 敵の移動方向
enum class ArrowEnemyDirection 
{
	kLeft,
	kRight,
};

// 敵の行動状態
enum class ArrowEnemyBehavior 
{
	kWalk,     // 巡回
	kAim,      // 狙いをつける
	kShoot,    // 発射
	kRecovery, // 射撃後の待機
};

class ArrowEnemy
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* arrowModel, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const Player* player);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition() const;

	// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField) 
	{ 
		mapChipField_ = mapChipField;
	}

	// 攻撃を受けた
	void OnHit(int damage);

	// ガードされたときに押し返される
	void PushBack(float moveX);

	// 死亡しているか
	bool IsDead() const
	{ 
		return isDead_ || isDeathAnimation_;
	}

	// 現在のHPを取得
	int GetHP() const
	{ 
		return hp_;
	}

	// 現在の攻撃が命中済みか
	bool HasAttackHit() const 
	{
		return hasAttackHit_;
	}

	// 現在の攻撃を命中済みにする
	void SetAttackHit()
	{
		hasAttackHit_ = true;
	}

	// このフレームに弾を発射するか
	bool IsShootRequested() const
	{ 
		return shootRequested_;
	}

	// 発射要求を消費する
	void ClearShootRequest()
	{
		shootRequested_ = false;
	}

	// 弾の発射位置を取得
	KamataEngine::Vector3 GetBulletSpawnPosition() const;

	// 発射方向を取得
	ArrowEnemyDirection GetDirection() const
	{
		return direction_;
	}

private:
	// マップ衝突判定の結果
	struct CollisionMapInfo 
	{
		// 地面へ着地したか
		bool landing = false;

		// 壁へ衝突したか
		bool hitWall = false;

		// 衝突を考慮した移動量
		KamataEngine::Vector3 move = {};
	};

	// 敵の四隅
	enum Corner 
	{
		kRightBottom,
		kLeftBottom,
		kRightTop,
		kLeftTop,

		kNumCorner
	};

private:
	// 移動処理
	void Move();

	// マップとの衝突判定
	void MapCollision(CollisionMapInfo& info);

	// 下方向の衝突判定
	void MapCollisionDown(CollisionMapInfo& info);

	// 左方向の衝突判定
	void MapCollisionLeft(CollisionMapInfo& info);

	// 右方向の衝突判定
	void MapCollisionRight(CollisionMapInfo& info);

	// 衝突判定結果を反映
	void ApplyCollisionResult(const CollisionMapInfo& info);

	// 接地状態を更新
	void UpdateOnGround(const CollisionMapInfo& info);

	// 四隅の座標を計算
	std::array<KamataEngine::Vector3, kNumCorner> GetCornerPositions(const KamataEngine::Vector3& center) const;

	// 敵の向きを反転
	void ReverseDirection();

	// 行動状態を更新
	void UpdateBehavior();

	// 攻撃対象が攻撃範囲内にいるか
	bool IsPlayerInAttackRange() const;

	// 行動状態を切り替える
	void ChangeBehavior(ArrowEnemyBehavior behavior);

private:
	// ワールド変換データ
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// マップチップフィールド
	MapChipField* mapChipField_ = nullptr;

	// 初期位置
	KamataEngine::Vector3 startPosition_ = {};

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 移動方向
	ArrowEnemyDirection direction_ = ArrowEnemyDirection::kRight;

	// 接地しているか
	bool onGround_ = false;

	// 移動速度
	static inline const float kWalkSpeed = 0.03f;

	// 初期位置からの巡回範囲
	static inline const float kWalkRange = 3.0f;

	// 重力
	static inline const float kGravity = 0.02f;

	// 最大落下速度
	static inline const float kLimitFallSpeed = 0.5f;

	// 敵の横幅
	static inline const float kWidth = 0.8f;

	// 敵の縦幅
	static inline const float kHeight = 0.8f;

	// 死亡しているか
	bool isDead_ = false;

	// 最大HP
	static inline const int kMaxHP = 2;

	// 現在のHP
	int hp_ = kMaxHP;

	// 怯み時間
	float stunTimer_ = 0.0f;

	// ガードされたときの怯み時間
	static inline const float kStunDuration = 0.2f;

	// 武器モデル
	KamataEngine::Model* arrowModel_ = nullptr;

	// 武器のワールド変換
	KamataEngine::WorldTransform arrowWorldTransform_;

	// 攻撃対象のプレイヤー
	const Player* player_ = nullptr;

	// 現在の行動状態
	ArrowEnemyBehavior behavior_ = ArrowEnemyBehavior::kWalk;

	// 現在の行動時間
	float behaviorTimer_ = 0.0f;

	// 構え時間
	static inline const float kWindupDuration = 0.15f;

	// 現在の攻撃が命中済みか
	bool hasAttackHit_ = false;

	// 射撃を開始する距離
	static inline const float kAttackStartDistance = 8.0f;

	// 狙いをつける時間
	static inline const float kAimDuration = 0.6f;

	// 射撃後の待機時間
	static inline const float kRecoveryDuration = 1.0f;

	// 弾の発射要求
	bool shootRequested_ = false;

	// 進行方向の先に床があるか
	bool IsGroundAhead() const;

	// 死亡演出中か
	bool isDeathAnimation_ = false;

	// 死亡演出の経過時間
	float deathAnimationTimer_ = 0.0f;

	// 死亡演出時間
	static inline const float kDeathAnimationDuration = 0.8f;

	// 死亡時の回転量
	static inline const float kDeathRotationSpeed = 0.15f;

	// 死亡時の沈み速度
	static inline const float kDeathFallSpeed = 0.02f;

	// 死亡演出を更新
	void UpdateDeathAnimation();

	// 被弾時のオブジェクトカラー
	KamataEngine::ObjectColor damageObjectColor_;

	// 被弾時の赤表示タイマー
	float damageFlashTimer_ = 0.0f;

	// 赤く表示する時間
	static inline const float kDamageFlashDuration = 0.1f;

};