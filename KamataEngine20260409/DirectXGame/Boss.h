#pragma once
#include "KamataEngine.h"
#include <numbers>

class Player;
class MapChipField;

enum class BossDirection 
{
	kLeft,
	kRight,
};

enum class BossBehavior 
{
	kIdle,

	kSwordWindup,
	kSwordAttack,
	kSwordRecovery,

	kSpinWindup,
	kSpinAttack,
	kSpinRecovery,

	kShootWindup,
	kShoot,
	kShootRecovery,

	kChargeWindup,
	kCharge,
	kChargeRecovery,
};

enum class BossPhase
{
	kPhase1,
	kPhase2,
	kPhase3,
};

class Boss
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Model* swordModel, KamataEngine::Model* crossbowModel, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const Player* player);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ワールド座標取得
	KamataEngine::Vector3 GetWorldPosition() const;

	// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField)
	{
		mapChipField_ = mapChipField;
	}

	// 死亡しているか
	bool IsDead() const
	{ 
		return isDead_ || isDeathAnimation_;
	}

	// HP取得
	int GetHP() const
	{ 
		return hp_; 
	}

	// 攻撃を受ける
	void OnHit(int damage);

	// 指定位置がボスの正面側か
	bool IsPositionInFront(const KamataEngine::Vector3& position) const;

	// 剣・回転攻撃の判定
	bool IsPositionInAttackRange(const KamataEngine::Vector3& position) const;

	// 指定位置が回転攻撃の範囲内か
	bool IsPositionInSpinAttackRange(const KamataEngine::Vector3& position) const;

	// 攻撃が命中済みか
	bool HasAttackHit() const
	{ 
		return hasAttackHit_;
	}

	void SetAttackHit()
	{
		hasAttackHit_ = true;
	}

	// 射撃要求
	bool IsShootRequested() const
	{ 
		return shootRequested_; 
	}

	void ClearShootRequest()
	{
		shootRequested_ = false;
	}

	// 弾の発射位置
	KamataEngine::Vector3 GetBulletSpawnPosition() const;

	// 向き
	BossDirection GetDirection() const
	{ 
		return direction_; 
	}

	// 突進中か
	bool IsCharging() const { return behavior_ == BossBehavior::kCharge; }

	// 突進を終了して硬直へ移行
	void StopCharge();

	void UpdatePhase();

	

private:
	// ボス本体
	KamataEngine::WorldTransform worldTransform_;

	// 剣
	KamataEngine::WorldTransform swordWorldTransform_;

	// クロスボウ
	KamataEngine::WorldTransform crossbowWorldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;
	KamataEngine::Model* swordModel_ = nullptr;
	KamataEngine::Model* crossbowModel_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// プレイヤー
	const Player* player_ = nullptr;

	// マップ
	MapChipField* mapChipField_ = nullptr;

	// 行動更新
	void UpdateBehavior();

	// 行動切り替え
	void ChangeBehavior(BossBehavior behavior);

	// プレイヤーが近距離か
	bool IsPlayerInCloseRange() const;

	// プレイヤーが正面にいるか
	bool IsPlayerInFront() const;

	// 向きをプレイヤー側へ変更
	void FacePlayer();

	// 向き
	BossDirection direction_ = BossDirection::kLeft;

	// 行動
	BossBehavior behavior_ = BossBehavior::kIdle;

	// 行動時間
	float behaviorTimer_ = 0.0f;

	// HP
	static inline const int kMaxHP = 10;
	int hp_ = kMaxHP;

	bool isDead_ = false;

	// 攻撃命中済み
	bool hasAttackHit_ = false;

	// 射撃要求
	bool shootRequested_ = false;

	// 3連射の発射回数
	int shootCount_ = 0;

	// 次の発射までの時間
	float shootIntervalTimer_ = 0.0f;

	// 剣の角度
	float swordAttackAngle_ = 0.0f;

    // 回転攻撃用の剣Y軸角度
	float spinSwordAngleY_ = 0.0f;

	// 本体の回転攻撃角度
	float spinAngle_ = 0.0f;

	// 基本距離
	static inline const float kCloseRange = 3.5f;

	// 時間
	static inline const float kSwordWindupDuration = 0.3f;
	static inline const float kSwordAttackDuration = 0.3f;

	static inline const float kSpinWindupDuration = 0.4f;
	static inline const float kSpinAttackDuration = 0.5f;

	static inline const float kShootWindupDuration = 0.4f;
	static inline const float kShootInterval = 0.2f;

	static inline const float kChargeWindupDuration = 0.4f;

	static inline const float kRecoveryDuration = 0.4f;

	// 剣を振る最大角度
	static inline const float kSwordAttackAngle = 2.5f;

	// 剣攻撃判定を置く前方距離
	static inline const float kSwordHitBoxOffsetX = 1.2f;

	// 剣攻撃判定の横半径
	static inline const float kSwordHitBoxHalfWidth = 0.9f;

	// 剣攻撃判定の縦半径
	static inline const float kSwordHitBoxHalfHeight = 1.0f;

	// 攻撃判定が有効になる進行度
	static inline const float kSwordActiveStart = 0.3f;
	static inline const float kSwordActiveEnd = 0.8f;

	// プレイヤーを探知する距離
	static inline const float kDetectionRange = 10.0f;

	// プレイヤーを探知範囲内に捉えているか
	bool IsPlayerDetected() const;

	// 剣攻撃中の踏み込み速度
	static inline const float kSwordAttackMoveSpeed = 0.08f;

	// 回転攻撃中に本体を1周させる角度
	static inline const float kSpinAttackAngle = 2.0f * 3.1415926535f;

	// 回転攻撃の前方判定
	static inline const float kSpinFrontHalfWidth = 1.5f;

	// 回転攻撃の後方判定
	static inline const float kSpinBackHalfWidth = 2.2f;

	// 回転攻撃の上下判定
	static inline const float kSpinHitBoxHalfHeight = 1.2f;

	// 回転攻撃の有効時間
	static inline const float kSpinActiveStart = 0.15f;
	static inline const float kSpinActiveEnd = 0.85f;

	// 回転攻撃前の剣の構え角度
	static inline const float kSpinSwordReadyAngle = std::numbers::pi_v<float> / 2.0f;

	// 突進開始位置
	KamataEngine::Vector3 chargeStartPosition_ = {};

	// 突進速度
	static inline const float kChargeSpeed = 0.16f;

	// 突進できる最大距離
	static inline const float kChargeMaxDistance = 6.0f;

	// ボス本体の描画専用Transform
	KamataEngine::WorldTransform drawWorldTransform_;

	// 突進時の壁判定
	bool CheckChargeWallCollision(float moveX);

	// 現在のフェーズ
	BossPhase phase_ = BossPhase::kPhase1;

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