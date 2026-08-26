#pragma once
#include "KamataEngine.h"
#include <array>

class MapChipField;
class Player;

// 敵の移動方向
enum class ShieldEnemyDirection
{
	kLeft,
	kRight,
};

// 敵の行動状態
enum class ShieldEnemyBehavior 
{
	kWalk,      // 巡回
	kWindup,    // 突進前の溜め
	kCharge,    // 突進
	kRecovery,  // 突進後の硬直
	kGuardStun, // 攻撃を盾で受けた後の怯み
};

class ShieldEnemy
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* shieldModel, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const Player* player);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition() const;

	// 指定した座標が盾の正面にあるか
	bool IsPositionInFront(const KamataEngine::Vector3& position) const;

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

	// 現在突進中か
	bool IsCharging() const 
	{ 
		return behavior_ == ShieldEnemyBehavior::kCharge;
	}

	// 突進を終了して硬直状態にする
	void StopCharge();

	// 正面から攻撃を防いだときの反応
	void OnGuard();

	// シャッタードアとの衝突を解決
	void ResolveShutterDoorCollision(const KamataEngine::Vector3& doorPosition);

	// MoveBlockとの衝突を解決
	void ResolveMoveBlockCollision(const KamataEngine::Vector3& moveBlockPosition, const KamataEngine::Vector3& moveBlockMoveAmount);

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

	// 行動状態を切り替える
	void ChangeBehavior(ShieldEnemyBehavior behavior);

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
	ShieldEnemyDirection direction_ = ShieldEnemyDirection::kRight;

	// 接地しているか
	bool onGround_ = false;

	// 移動速度
	static inline const float kWalkSpeed = 0.03f;

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
	static inline const float kStunDuration = 1.0f;

	// ガード成功後の怯み時間
	static inline const float kGuardStunDuration = 1.0f;

	// 盾モデル
	KamataEngine::Model* shieldModel_ = nullptr;

	// 盾のワールド変換
	KamataEngine::WorldTransform shieldWorldTransform_;

	// 敵本体の描画専用ワールド変換
	KamataEngine::WorldTransform drawWorldTransform_;

	// 攻撃対象のプレイヤー
	const Player* player_ = nullptr;

	// 現在の行動状態
	ShieldEnemyBehavior behavior_ = ShieldEnemyBehavior::kWalk;

	// 現在の行動時間
	float behaviorTimer_ = 0.0f;

	// 盾の回転角度
	float shieldAttackAngle_ = 0.0f;

	// 突進開始位置
	KamataEngine::Vector3 chargeStartPosition_ = {};

	// 突進開始距離
	static inline const float kChargeStartDistance = 6.0f;

	// 突進前の溜め時間
	static inline const float kWindupDuration = 0.6f;

	// 突進速度
	static inline const float kChargeSpeed = 0.12f;

	// 突進できる最大距離
	static inline const float kChargeMaxDistance = 5.0f;

	// 突進後の硬直時間
	static inline const float kRecoveryDuration = 0.8f;

	// プレイヤーが正面の突進範囲内にいるか
	bool IsPlayerInChargeRange() const;

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

	// 通常歩行中、前方に壁または崖があるか
	bool ShouldReverseWalkDirection() const;

	// 更新前の敵位置
	KamataEngine::Vector3 previousPosition_ = {};

};