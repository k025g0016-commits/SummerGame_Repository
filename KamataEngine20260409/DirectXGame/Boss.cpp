#include "Boss.h"
#include "MathUtility.h"
#include "MapChipField.h"
#include "Player.h"
#include <cassert>
#include <numbers>
#include <cmath>

using namespace KamataEngine;

void Boss::Initialize(Model* model, Model* swordModel, Model* crossbowModel, Camera* camera, const Vector3& position, const Player* player) 
{
	assert(model);
	assert(swordModel);
	assert(crossbowModel);
	assert(camera);
	assert(player);

	model_ = model;
	swordModel_ = swordModel;
	crossbowModel_ = crossbowModel;
	camera_ = camera;
	player_ = player;

	worldTransform_.Initialize();
	swordWorldTransform_.Initialize();
	crossbowWorldTransform_.Initialize();
	drawWorldTransform_.Initialize();
	damageObjectColor_.Initialize();

	// 被弾時に使う赤色
	damageObjectColor_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});

	worldTransform_.translation_ = position;

	// とりあえず左向き
	worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;

	// 武器は最初は本体と同じ位置・向き
	swordWorldTransform_.translation_ = worldTransform_.translation_;

	swordWorldTransform_.rotation_ = worldTransform_.rotation_;

	crossbowWorldTransform_.translation_ = worldTransform_.translation_;

	crossbowWorldTransform_.rotation_ = worldTransform_.rotation_;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	swordWorldTransform_.matWorld_ = MakeAffineMatrix(swordWorldTransform_.scale_, swordWorldTransform_.rotation_, swordWorldTransform_.translation_);

	swordWorldTransform_.TransferMatrix();

	crossbowWorldTransform_.matWorld_ = MakeAffineMatrix(crossbowWorldTransform_.scale_, crossbowWorldTransform_.rotation_, crossbowWorldTransform_.translation_);

	crossbowWorldTransform_.TransferMatrix();

	swordAttackRequested_ = false;

}

void Boss::Update()
{
	if (isDead_)
	{
		return;
	}

	if (isDeathAnimation_)
	{
		UpdateDeathAnimation();
		return;
	}

	// 被弾時の赤表示タイマー
	if (damageFlashTimer_ > 0.0f) 
	{
		damageFlashTimer_ -= 1.0f / 60.0f;

		if (damageFlashTimer_ < 0.0f)
		{
			damageFlashTimer_ = 0.0f;
		}
	}

	UpdatePhase();
	UpdateBehavior();

	// =========================
	// ボス本体の向きを決める
	// =========================
	float baseRotationY = 0.0f;

	if (direction_ == BossDirection::kRight) 
	{
		baseRotationY = std::numbers::pi_v<float> / 2.0f;
	}
	else
	{
		baseRotationY = -std::numbers::pi_v<float> / 2.0f;
	}

	// 回転攻撃の角度を加える
	worldTransform_.rotation_.y = baseRotationY + spinAngle_;

	// =========================
	// 剣を本体へ追従
	// =========================
	swordWorldTransform_.translation_ = worldTransform_.translation_;

	swordWorldTransform_.rotation_ = worldTransform_.rotation_;

	// 通常の剣攻撃はZ軸回転
	if (direction_ == BossDirection::kRight)
	{
		swordWorldTransform_.rotation_.z += swordAttackAngle_;
	} else {
		swordWorldTransform_.rotation_.z -= swordAttackAngle_;
	}

	// 回転攻撃時はY軸方向へ剣を横向きにする
	swordWorldTransform_.rotation_.y += spinSwordAngleY_;

	// =========================
	// クロスボウを本体へ追従
	// =========================
	crossbowWorldTransform_.translation_ = worldTransform_.translation_;

	crossbowWorldTransform_.rotation_ = worldTransform_.rotation_;

	// =========================
	// 行列更新
	// =========================
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	swordWorldTransform_.matWorld_ = MakeAffineMatrix(swordWorldTransform_.scale_, swordWorldTransform_.rotation_, swordWorldTransform_.translation_);

	swordWorldTransform_.TransferMatrix();

	crossbowWorldTransform_.matWorld_ = MakeAffineMatrix(crossbowWorldTransform_.scale_, crossbowWorldTransform_.rotation_, crossbowWorldTransform_.translation_);

	crossbowWorldTransform_.TransferMatrix();
}

void Boss::Draw() 
{
	if (isDead_ || model_ == nullptr || swordModel_ == nullptr || crossbowModel_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	// 通常時は本体と同じ
	drawWorldTransform_.scale_ = worldTransform_.scale_;

	drawWorldTransform_.rotation_ = worldTransform_.rotation_;

	drawWorldTransform_.translation_ = worldTransform_.translation_;

	// 突進前の溜め
	if (behavior_ == BossBehavior::kChargeWindup)
	{
		// 上下へ少し伸びる
		drawWorldTransform_.scale_.z = 0.75f;
		drawWorldTransform_.scale_.y = 1.15f;
	}
	// 突進中
	else if (behavior_ == BossBehavior::kCharge)
	{
		// 横方向へ勢いよく伸びる印象
		drawWorldTransform_.scale_.z = 1.2f;
		drawWorldTransform_.scale_.y = 0.85f;
	}

	drawWorldTransform_.matWorld_ = MakeAffineMatrix(drawWorldTransform_.scale_, drawWorldTransform_.rotation_, drawWorldTransform_.translation_);

	drawWorldTransform_.TransferMatrix();

	// 敵本体
	if (damageFlashTimer_ > 0.0f && !isDeathAnimation_) 
	{
		// 被弾中は赤色
		model_->Draw(drawWorldTransform_, *camera_, &damageObjectColor_);
	}
	else 
	{
		// 通常色
		model_->Draw(drawWorldTransform_, *camera_);
	}

	swordModel_->Draw(swordWorldTransform_, *camera_);

	crossbowModel_->Draw(crossbowWorldTransform_, *camera_);
}

Vector3 Boss::GetWorldPosition() const 
{ 
	return worldTransform_.translation_;
}

void Boss::FacePlayer() 
{
	if (player_ == nullptr)
	{
		return;
	}

	const Vector3 playerPosition = player_->GetWorldPosition();

	if (playerPosition.x >= worldTransform_.translation_.x) 
	{
		direction_ = BossDirection::kRight;
	}
	else 
	{
		direction_ = BossDirection::kLeft;
	}
}

bool Boss::IsPlayerInCloseRange() const
{
	if (player_ == nullptr)
	{
		return false;
	}

	const Vector3 playerPosition = player_->GetWorldPosition();

	const float distanceX = std::abs(playerPosition.x - worldTransform_.translation_.x);

	return distanceX <= kCloseRange;
}

bool Boss::IsPlayerInFront() const
{
	if (player_ == nullptr)
	{
		return false;
	}

	const Vector3 playerPosition = player_->GetWorldPosition();

	if (direction_ == BossDirection::kRight) 
	{
		return playerPosition.x >= worldTransform_.translation_.x;
	}

	return playerPosition.x <= worldTransform_.translation_.x;
}

void Boss::ChangeBehavior(BossBehavior behavior)
{
	behavior_ = behavior;

	behaviorTimer_ = 0.0f;

	hasAttackHit_ = false;

	shootRequested_ = false;

	shootCount_ = 0;

	shootIntervalTimer_ = 0.0f;
}

void Boss::UpdateBehavior()
{
	const float deltaTime = 1.0f / 60.0f;

	behaviorTimer_ += deltaTime;

	switch (behavior_)
	{
	case BossBehavior::kIdle: 
	{
		if (!IsPlayerDetected()) 
		{
			break;
		}

		if (IsPlayerInCloseRange())
		{
			if (IsPlayerInFront()) 
			{
				ChangeBehavior(BossBehavior::kSwordWindup);
			}
			else
			{
				ChangeBehavior(BossBehavior::kSpinWindup);
			}
		} 
		else
		{
			FacePlayer();

			switch (phase_) 
			{
			case BossPhase::kPhase1:

				// 突進のみ
				ChangeBehavior(BossBehavior::kChargeWindup);

				break;

			case BossPhase::kPhase2:

				// 突進か3連射
				if (std::rand() % 2 == 0) 
				{
					ChangeBehavior(BossBehavior::kShootWindup);
				}
				else 
				{
					ChangeBehavior(BossBehavior::kChargeWindup);
				}

				break;

			case BossPhase::kPhase3:

				// 突進のみ
				ChangeBehavior(BossBehavior::kChargeWindup);

				break;
			}
		}

		break;
	}

		case BossBehavior::kSwordWindup:
		{
		float progress = behaviorTimer_ / kSwordWindupDuration;

		if (progress > 1.0f) 
		{
			progress = 1.0f;
		}

		// 攻撃前に剣を後ろへ引く
		swordAttackAngle_ = kSwordAttackAngle * 0.5f * progress;

		if (behaviorTimer_ >= kSwordWindupDuration)
		{
			ChangeBehavior(BossBehavior::kSwordAttack);

			// 剣を振り始めたのでSE再生要求
			swordAttackRequested_ = true;

		}

		break;
	}

	case BossBehavior::kSwordAttack:
	{
		float progress = behaviorTimer_ / kSwordAttackDuration;

		if (progress > 1.0f)
		{
			progress = 1.0f;
		}

		// 後方から前方へ大きく振る
		swordAttackAngle_ = kSwordAttackAngle * 0.5f - kSwordAttackAngle * 1.5f * progress;

		// 剣攻撃中は前進する
		if (direction_ == BossDirection::kRight)
		{
			worldTransform_.translation_.x += kSwordAttackMoveSpeed;
		}
		else 
		{
			worldTransform_.translation_.x -= kSwordAttackMoveSpeed;
		}

	if (behaviorTimer_ >= kSwordAttackDuration)
	{
			ChangeBehavior(BossBehavior::kSwordRecovery);
		}

		break;
	}

	case BossBehavior::kSwordRecovery:
	{
		float progress = behaviorTimer_ / kRecoveryDuration;

		if (progress > 1.0f) 
		{
			progress = 1.0f;
		}

		swordAttackAngle_ = -kSwordAttackAngle * (1.0f - progress);

		if (behaviorTimer_ >= kRecoveryDuration)
		{
			swordAttackAngle_ = 0.0f;

			ChangeBehavior(BossBehavior::kIdle);
		}

		break;
	}

	case BossBehavior::kShootWindup: 
	{
		// 射撃前の構え時間
		if (behaviorTimer_ >= kShootWindupDuration)
		{
			ChangeBehavior(BossBehavior::kShoot);
		}

		break;
	}

	case BossBehavior::kShoot:
	{
		// 次の発射までの時間を進める
		shootIntervalTimer_ += deltaTime;

		// まだ3発撃っておらず、
		// 発射間隔を過ぎたら射撃要求を出す
		if (shootCount_ < 3 && shootIntervalTimer_ >= kShootInterval) 
		{
			// GameSceneへ弾生成を要求
			shootRequested_ = true;

			// 発射回数を増やす
			++shootCount_;

			// 次弾用にタイマーをリセット
			shootIntervalTimer_ = 0.0f;
		}

		// 3発撃ち終わったら硬直へ
		if (shootCount_ >= 3 && !shootRequested_) 
		{
			ChangeBehavior(BossBehavior::kShootRecovery);
		}

		break;
	}

	case BossBehavior::kShootRecovery: 
	{
		if (behaviorTimer_ >= kRecoveryDuration)
		{
			ChangeBehavior(BossBehavior::kIdle);
		}

		break;
	}

	case BossBehavior::kChargeWindup:
	{
		// 突進前の溜め
		if (behaviorTimer_ >= kChargeWindupDuration)
		{
			// 突進開始位置を記録
			chargeStartPosition_ = worldTransform_.translation_;

			ChangeBehavior(BossBehavior::kCharge);
		}

		break;
	}

	case BossBehavior::kCharge:
	{
		float moveX = 0.0f;

		if (direction_ == BossDirection::kRight) 
		{
			moveX = kChargeSpeed;
		}
		else 
		{
			moveX = -kChargeSpeed;
		}

		// 移動する前に壁を確認
		if (CheckChargeWallCollision(moveX))
		{
			// 壁に当たったので突進終了
			ChangeBehavior(BossBehavior::kChargeRecovery);

			break;
		}

		// 壁がなければ移動
		worldTransform_.translation_.x += moveX;

		const Vector3 difference = Subtract(worldTransform_.translation_, chargeStartPosition_);

		const float distance = Length(difference);

		// 最大距離に到達したら突進終了
		if (distance >= kChargeMaxDistance)
		{
			ChangeBehavior(BossBehavior::kChargeRecovery);
		}

		break;
	}

	case BossBehavior::kChargeRecovery:
	{
		if (behaviorTimer_ >= kRecoveryDuration) 
		{
			if (phase_ == BossPhase::kPhase3)
			{
				ChangeBehavior(BossBehavior::kSpinWindup);
			}
			else
			{
				ChangeBehavior(BossBehavior::kIdle);
			}
		}

		break;
	}

	case BossBehavior::kSpinWindup: 
	{
		float progress = behaviorTimer_ / kSpinWindupDuration;

		if (progress > 1.0f)
		{
			progress = 1.0f;
		}

		// 剣を徐々に横向きへ構える
		spinSwordAngleY_ = kSpinSwordReadyAngle * progress;

		if (behaviorTimer_ >= kSpinWindupDuration)
		{
			spinAngle_ = 0.0f;

			ChangeBehavior(BossBehavior::kSpinAttack);

			// 回転斬り開始
			swordAttackRequested_ = true;

		}

		break;
	}

	case BossBehavior::kSpinAttack:
	{
		float progress = behaviorTimer_ / kSpinAttackDuration;

		if (progress > 1.0f) 
		{
			progress = 1.0f;
		}

		// 剣は横向きに固定
		spinSwordAngleY_ = kSpinSwordReadyAngle;

		// 本体を1回転
		spinAngle_ = kSpinAttackAngle * progress;

		if (behaviorTimer_ >= kSpinAttackDuration)
		{
			spinAngle_ = 0.0f;

			// 回転終了後にプレイヤー方向を向く
			FacePlayer();

			ChangeBehavior(BossBehavior::kSpinRecovery);
		}

		break;
	}

	case BossBehavior::kSpinRecovery: 
	{
		float progress = behaviorTimer_ / kRecoveryDuration;

		if (progress > 1.0f) 
		{
			progress = 1.0f;
		}

		// 横向きだった剣を元の位置へ戻す
		spinSwordAngleY_ = kSpinSwordReadyAngle * (1.0f - progress);

		if (behaviorTimer_ >= kRecoveryDuration)
		{
			spinSwordAngleY_ = 0.0f;

			ChangeBehavior(BossBehavior::kIdle);
		}

		break;
	}

	default:

		break;
	}
}

bool Boss::IsPositionInAttackRange(const Vector3& position) const
{
	// 剣攻撃中でなければ無効
	if (behavior_ != BossBehavior::kSwordAttack)
	{
		return false;
	}

	if (hasAttackHit_)
	{
		return false;
	}

	const float progress = behaviorTimer_ / kSwordAttackDuration;

	// 振り始め・振り終わりには当たり判定を出さない
	if (progress < kSwordActiveStart || progress > kSwordActiveEnd)
	{
		return false;
	}

	const Vector3 bossPosition = worldTransform_.translation_;

	float hitBoxCenterX = bossPosition.x;

	if (direction_ == BossDirection::kRight) 
	{
		hitBoxCenterX += kSwordHitBoxOffsetX;
	}
	else
	{
		hitBoxCenterX -= kSwordHitBoxOffsetX;
	}

	const float distanceX = std::abs(position.x - hitBoxCenterX);

	const float distanceY = std::abs(position.y - bossPosition.y);

	return distanceX <= kSwordHitBoxHalfWidth && distanceY <= kSwordHitBoxHalfHeight;
}

bool Boss::IsPlayerDetected() const 
{
	if (player_ == nullptr || player_->IsDead()) 
	{
		return false;
	}

	const Vector3 playerPosition = player_->GetWorldPosition();

	const float distanceX = std::abs(playerPosition.x - worldTransform_.translation_.x);

	const float distanceY = std::abs(playerPosition.y - worldTransform_.translation_.y);

	// 横方向を主に見る。
	// 高さが極端に違う相手には反応しないようにする
	constexpr float kDetectionHeight = 3.0f;

	return distanceX <= kDetectionRange && distanceY <= kDetectionHeight;
}

bool Boss::IsPositionInFront(const Vector3& position) const 
{
	if (direction_ == BossDirection::kRight) 
	{
		// 右向きなら右側が正面
		return position.x >= worldTransform_.translation_.x;
	}
	else
	{
		// 左向きなら左側が正面
		return position.x <= worldTransform_.translation_.x;
	}
}

void Boss::OnHit(int damage)
{
	if (isDead_ || isDeathAnimation_) 
	{
		return;
	}

	if (damage <= 0)
	{
		return;
	}

	// 被弾時の赤表示開始
	damageFlashTimer_ = kDamageFlashDuration;

	hp_ -= damage;

	if (hp_ > 0)
	{
		return;
	}

	hp_ = 0;

	// 死亡演出開始
	isDeathAnimation_ = true;
	deathAnimationTimer_ = 0.0f;

}

bool Boss::IsPositionInSpinAttackRange(const Vector3& position) const
{
	if (behavior_ != BossBehavior::kSpinAttack) 
	{
		return false;
	}

	if (hasAttackHit_) 
	{
		return false;
	}

	const float progress = behaviorTimer_ / kSpinAttackDuration;

	if (progress < kSpinActiveStart || progress > kSpinActiveEnd)
	{
		return false;
	}

	const Vector3 bossPosition = worldTransform_.translation_;

	const float offsetX = position.x - bossPosition.x;

	const float distanceY = std::abs(position.y - bossPosition.y);

	float halfWidth = 0.0f;

	if (direction_ == BossDirection::kRight)
	{
		// 右を向いている場合
		if (offsetX >= 0.0f)
		{
			halfWidth = kSpinFrontHalfWidth;
		}
		else
		{
			halfWidth = kSpinBackHalfWidth;
		}
	} 
	else
	{
		// 左を向いている場合
		if (offsetX <= 0.0f) 
		{
			halfWidth = kSpinFrontHalfWidth;
		}
		else
		{
			halfWidth = kSpinBackHalfWidth;
		}
	}

	return std::abs(offsetX) <= halfWidth && distanceY <= kSpinHitBoxHalfHeight;
}

Vector3 Boss::GetBulletSpawnPosition() const 
{
	Vector3 position = worldTransform_.translation_;

	// クロスボウの位置から少し前へ出す
	constexpr float kSpawnOffsetX = 1.2f;

	// 少し上側から発射
	constexpr float kSpawnOffsetY = 0.2f;

	if (direction_ == BossDirection::kRight)
	{
		position.x += kSpawnOffsetX;
	}
	else 
	{
		position.x -= kSpawnOffsetX;
	}

	position.y += kSpawnOffsetY;

	return position;
}

void Boss::StopCharge() 
{
	if (behavior_ == BossBehavior::kCharge)
	{
		ChangeBehavior(BossBehavior::kChargeRecovery);
	}
}

bool Boss::CheckChargeWallCollision(float moveX)
{
	if (mapChipField_ == nullptr) 
	{
		return false;
	}

	// ボスの横方向の当たり判定
	constexpr float kBossHalfWidth = 1.0f;

	// 次のフレームでのボス中心位置
	Vector3 nextPosition = worldTransform_.translation_;
	nextPosition.x += moveX;

	// 進行方向側の端まで移動
	if (moveX > 0.0f) 
	{
		nextPosition.x += kBossHalfWidth;
	} else {
		nextPosition.x -= kBossHalfWidth;
	}

	// =========================
	// 壁判定用の2点
	// =========================

	// ボスの中心付近
	Vector3 checkCenter = nextPosition;

	// ボスの上側
	Vector3 checkTop = nextPosition;
	checkTop.y += 0.8f;

	// 中央のマップチップ
	const MapChipIndexSet centerIndex = mapChipField_->GetMapChipIndexSetByPosition(checkCenter);

	// 上側のマップチップ
	const MapChipIndexSet topIndex = mapChipField_->GetMapChipIndexSetByPosition(checkTop);

	const bool hitCenter = mapChipField_->GetMapChipTypeByIndex(centerIndex.xIndex, centerIndex.yIndex) == MapChipType::kBlock;

	const bool hitTop = mapChipField_->GetMapChipTypeByIndex(topIndex.xIndex, topIndex.yIndex) == MapChipType::kBlock;

	// 中央か上側のどちらかがB0なら壁
	return hitCenter || hitTop;
}

void Boss::UpdatePhase() 
{
	float hpRate = static_cast<float>(hp_) / static_cast<float>(kMaxHP);

	if (hpRate > 0.75f) 
	{
		phase_ = BossPhase::kPhase1;
	} 
	else if (hpRate > 0.25f) 
	{
		phase_ = BossPhase::kPhase2;
	}
	else 
	{
		phase_ = BossPhase::kPhase3;
	}
}

void Boss::UpdateDeathAnimation()
{
	constexpr float kDeltaTime = 1.0f / 60.0f;

	deathAnimationTimer_ += kDeltaTime;

	// 回転
	worldTransform_.rotation_.y += kDeathRotationSpeed;

	// 少し沈む
	worldTransform_.translation_.y -= kDeathFallSpeed;

	// 徐々に縮小
	float progress = deathAnimationTimer_ / kDeathAnimationDuration;

	if (progress > 1.0f)
	{
		progress = 1.0f;
	}

	const float scale = 1.0f - progress;

	worldTransform_.scale_ = {scale, scale, scale};

	// 武器も追従
	swordWorldTransform_.translation_ = worldTransform_.translation_;
	swordWorldTransform_.rotation_ = worldTransform_.rotation_;
	swordWorldTransform_.scale_ = worldTransform_.scale_;

	crossbowWorldTransform_.translation_ = worldTransform_.translation_;
	crossbowWorldTransform_.rotation_ = worldTransform_.rotation_;
	crossbowWorldTransform_.scale_ = worldTransform_.scale_;

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	swordWorldTransform_.matWorld_ = MakeAffineMatrix(swordWorldTransform_.scale_, swordWorldTransform_.rotation_, swordWorldTransform_.translation_);

	swordWorldTransform_.TransferMatrix();

	crossbowWorldTransform_.matWorld_ = MakeAffineMatrix(crossbowWorldTransform_.scale_, crossbowWorldTransform_.rotation_, crossbowWorldTransform_.translation_);

	crossbowWorldTransform_.TransferMatrix();

	if (deathAnimationTimer_ >= kDeathAnimationDuration) 
	{
		isDeathAnimation_ = false;
		isDead_ = true;
	}
}