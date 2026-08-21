#pragma once
#include "KamataEngine.h"
#include <array>

class MapChipField;
class Player;

// 敵の移動方向
enum class EnemyDirection 
{
	kLeft,
	kRight,
};

// 敵の行動状態
enum class EnemyBehavior
{
	kWalk,      // 歩行
	kWindup,    // 攻撃前の構え
	kAttack,    // 踏み込み斬り
	kRecovery,  // 攻撃後の硬直
	kGuardStun, // ガードされた後ののけ反り
};

class Enemy 
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* swordModel, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const Player* player);

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

	// 剣の攻撃判定が有効か
	bool IsAttackActive() const;

	// 指定位置が剣の攻撃範囲内か
	bool IsPositionInAttackRange(const KamataEngine::Vector3& position) const;

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
	void ChangeBehavior(EnemyBehavior behavior);

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
	EnemyDirection direction_ = EnemyDirection::kRight;

	// 接地しているか
	bool onGround_ = false;

	// 移動速度
	static inline const float kWalkSpeed = 0.03f;

	// 進行方向の先に床があるか
	bool IsGroundAhead() const;

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

	// 剣モデル
	KamataEngine::Model* swordModel_ = nullptr;

	// 剣のワールド変換
	KamataEngine::WorldTransform swordWorldTransform_;

	// 攻撃対象のプレイヤー
	const Player* player_ = nullptr;

	// 現在の行動状態
	EnemyBehavior behavior_ = EnemyBehavior::kWalk;

	// 現在の行動時間
	float behaviorTimer_ = 0.0f;

	// 剣の攻撃角度
	float swordAttackAngle_ = 0.0f;

	// 攻撃を開始する距離
	static inline const float kAttackStartDistance = 2.0f;

	// 構え時間
	static inline const float kWindupDuration = 0.15f;

	// 攻撃時間
	static inline const float kAttackDuration = 0.18f;

	// 攻撃後の硬直時間
	static inline const float kRecoveryDuration = 0.25f;

	// 攻撃中の踏み込み速度
	static inline const float kAttackMoveSpeed = 0.08f;

	// 剣を振る最大角度
	static inline const float kAttackSwordAngle = 2.5f;

	// 現在の攻撃が命中済みか
	bool hasAttackHit_ = false;

	// 剣の攻撃判定を置く前方距離
	static inline const float kAttackHitBoxOffsetX = 0.8f;

	// 剣の攻撃判定の横半径
	static inline const float kAttackHitBoxHalfWidth = 0.6f;

	// 剣の攻撃判定の縦半径
	static inline const float kAttackHitBoxHalfHeight = 0.5f;

	// 攻撃判定が始まる進行度
	static inline const float kAttackActiveStart = 0.3f;

	// 攻撃判定が終わる進行度
	static inline const float kAttackActiveEnd = 0.8f;

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

	// ガードされた後ののけ反り時間
	static inline const float kGuardStunDuration = 0.6f;

	// 敵本体の描画専用Transform
	KamataEngine::WorldTransform drawWorldTransform_;

	// 剣の描画専用Transform
	KamataEngine::WorldTransform drawSwordWorldTransform_;

};