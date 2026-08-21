#include "Enemy.h"
#include "MapChipField.h"
#include "MathUtility.h"
#include <array>
#include <cassert>
#include <numbers>
#include "Player.h"
#include <cmath>

using namespace KamataEngine;

namespace 
{
   constexpr float kCollisionEpsilon = 0.01f;
}

void Enemy::Initialize(Model* model, Model* swordModel, Camera* camera, const Vector3& position, const Player* player) 
{
	// nullptrチェック
	assert(model);
	assert(swordModel);
	assert(camera);
	assert(player);

	model_ = model;
	swordModel_ = swordModel;
	camera_ = camera;
	player_ = player;

	// 必ずTransferMatrixより先に初期化する
	worldTransform_.Initialize();
	swordWorldTransform_.Initialize();
	drawWorldTransform_.Initialize();
	drawSwordWorldTransform_.Initialize();
	damageObjectColor_.Initialize();

	// 被弾時に使う赤色
	damageObjectColor_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});

	// 初期位置
	worldTransform_.translation_ = position;
	startPosition_ = position;

	// 速度を初期化
	velocity_ = {};

	// 最初は右へ移動
	direction_ = EnemyDirection::kRight;

	// 初期状態では未接地
	onGround_ = false;

	// モデルの大きさ
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 右向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 敵本体のワールド行列を更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 剣を敵本体と同じ位置・向き・大きさにする
	swordWorldTransform_.scale_ = worldTransform_.scale_;

	swordWorldTransform_.rotation_ = worldTransform_.rotation_;

	swordWorldTransform_.translation_ = worldTransform_.translation_;

	swordWorldTransform_.matWorld_ = MakeAffineMatrix(swordWorldTransform_.scale_, swordWorldTransform_.rotation_, swordWorldTransform_.translation_);

	swordWorldTransform_.TransferMatrix();

	// HPなどの初期化
	hp_ = kMaxHP;
	isDead_ = false;
	isDeathAnimation_ = false;
	deathAnimationTimer_ = 0.0f;
	damageFlashTimer_ = 0.0f;
	stunTimer_ = 0.0f;

	behavior_ = EnemyBehavior::kWalk;
	behaviorTimer_ = 0.0f;
	swordAttackAngle_ = 0.0f;

	hasAttackHit_ = false;

}

void Enemy::Update() 
{
	// 死亡している場合は更新しない
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

	// 怯み時間を減らす
	if (stunTimer_ > 0.0f) 
	{
		stunTimer_ -= 1.0f / 60.0f;

		if (stunTimer_ < 0.0f) 
		{
			stunTimer_ = 0.0f;
		}
	}

	// 怯み中でなければ行動状態を更新
	if (stunTimer_ <= 0.0f)
	{
		UpdateBehavior();
	}

	// 左右移動
	Move();

	// 重力
	velocity_.y -= kGravity;

	// 最大落下速度を制限
	if (velocity_.y < -kLimitFallSpeed)
	{
		velocity_.y = -kLimitFallSpeed;
	}

	// マップとの衝突判定
	CollisionMapInfo info{};

	MapCollision(info);
	ApplyCollisionResult(info);
	UpdateOnGround(info);

	// 壁に衝突した場合
	if (info.hitWall) 
	{
		// 巡回中だけ方向転換する
		if (behavior_ == EnemyBehavior::kWalk)
		{
			ReverseDirection();
		}

		// 攻撃中などは向きを変えない
	}

	// 巡回中に崖へ近づいたら方向転換
	if (behavior_ == EnemyBehavior::kWalk)
	{
		if (!IsGroundAhead()) 
		{
			ReverseDirection();
		}
	}

	// モデルの向きを更新
	if (direction_ == EnemyDirection::kRight)
	{
		worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	}
	else 
	{
		worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;
	}

	// ワールド行列を更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 剣を敵本体に追従させる
	swordWorldTransform_.scale_ = worldTransform_.scale_;
	swordWorldTransform_.rotation_ = worldTransform_.rotation_;
	swordWorldTransform_.translation_ = worldTransform_.translation_;

	// 攻撃モーション用の回転を追加
	if (direction_ == EnemyDirection::kRight) 
	{
		swordWorldTransform_.rotation_.z += swordAttackAngle_;
	} 
	else 
	{
		swordWorldTransform_.rotation_.z -= swordAttackAngle_;
	}

	swordWorldTransform_.matWorld_ = MakeAffineMatrix(swordWorldTransform_.scale_, swordWorldTransform_.rotation_, swordWorldTransform_.translation_);

	swordWorldTransform_.TransferMatrix();

}

void Enemy::Move() 
{
	// 怯み中は横移動しない
	if (stunTimer_ > 0.0f) 
	{
		velocity_.x = 0.0f;
		return;
	}

	// 攻撃前と攻撃後は停止
	if (behavior_ == EnemyBehavior::kWindup || behavior_ == EnemyBehavior::kRecovery || behavior_ == EnemyBehavior::kGuardStun) 
	{
		velocity_.x = 0.0f;
		return;
	}

	float moveSpeed = kWalkSpeed;

	// 攻撃中は前進しながら斬る
	if (behavior_ == EnemyBehavior::kAttack)
	{
		// 崖まで来たら前進だけ停止する
		// 向きは反転しない
		if (!IsGroundAhead())
		{
			velocity_.x = 0.0f;
			return;
		}

		moveSpeed = kAttackMoveSpeed;
	}

	if (direction_ == EnemyDirection::kRight) 
	{
		velocity_.x = moveSpeed;
	} 
	else
	{
		velocity_.x = -moveSpeed;
	}
}

void Enemy::ReverseDirection()
{
	if (direction_ == EnemyDirection::kRight) 
	{
		direction_ = EnemyDirection::kLeft;
	}
	else
	{
		direction_ = EnemyDirection::kRight;
	}
}

std::array<Vector3, Enemy::kNumCorner> Enemy::GetCornerPositions(const Vector3& center) const 
{
	std::array<Vector3, kNumCorner> positions;

	const float halfWidth = kWidth / 2.0f;
	const float halfHeight = kHeight / 2.0f;

	positions[kLeftBottom] = 
	{
		center.x - halfWidth,
		center.y - halfHeight,
		center.z
	};

	positions[kRightBottom] = 
	{
		center.x + halfWidth,
		center.y - halfHeight,
		center.z
	};

	positions[kLeftTop] = 
	{
		center.x - halfWidth,
		center.y + halfHeight,
		center.z
	};

	positions[kRightTop] =
	{
		center.x + halfWidth,
		center.y + halfHeight,
		center.z
	};

	return positions;
}

void Enemy::MapCollision(CollisionMapInfo& info) 
{
	// 現在の速度を移動量として設定
	info.move = velocity_;

	MapCollisionDown(info);
	MapCollisionLeft(info);
	MapCollisionRight(info);
}

void Enemy::MapCollisionDown(CollisionMapInfo& info)
{
	// 下方向へ移動していなければ判定しない
	if (info.move.y >= 0.0f)
	{
		return;
	}

	if (mapChipField_ == nullptr)
	{
		return;
	}

	const Vector3 currentCenter = worldTransform_.translation_;

	// Y方向だけ移動させる
	Vector3 movedCenter = currentCenter;
	movedCenter.y += info.move.y;

	const std::array<Vector3, kNumCorner> currentCorners = GetCornerPositions(currentCenter);

	const std::array<Vector3, kNumCorner> movedCorners = GetCornerPositions(movedCenter);

	const Corner checkCorners[] =
	{
		kLeftBottom,
		kRightBottom
	};

	bool isCollision = false;
	float highestBlockTop = 0.0f;

	for (Corner corner : checkCorners)
	{
		Vector3 currentCheckPosition = currentCorners[corner];

		Vector3 movedCheckPosition = movedCorners[corner];

		// ブロック側面を床として判定しないように内側へずらす
		if (corner == kLeftBottom)
		{
			currentCheckPosition.x += kCollisionEpsilon;
			movedCheckPosition.x += kCollisionEpsilon;
		} 
		else
		{
			currentCheckPosition.x -= kCollisionEpsilon;
			movedCheckPosition.x -= kCollisionEpsilon;
		}

		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCheckPosition);

		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCheckPosition);

		// マスの境界を越えていなければ判定しない
		if (currentIndex.yIndex == movedIndex.yIndex) 
		{
			continue;
		}

		if (mapChipField_->GetMapChipTypeByIndex(movedIndex.xIndex, movedIndex.yIndex) != MapChipType::kBlock)
		{
			continue;
		}

		const MapChipRect blockRect = mapChipField_->GetRectByIndex(movedIndex.xIndex, movedIndex.yIndex);

		if (!isCollision || blockRect.top > highestBlockTop) 
		{
			highestBlockTop = blockRect.top;
		}

		isCollision = true;
	}

	if (!isCollision) 
	{
		return;
	}

	// 敵の下端をブロック上端へ合わせる
	const float correctedCenterY = highestBlockTop + kHeight / 2.0f;

	info.move.y = correctedCenterY - worldTransform_.translation_.y;

	info.landing = true;
}

void Enemy::MapCollisionLeft(CollisionMapInfo& info)
{
	if (info.move.x >= 0.0f)
	{
		return;
	}

	if (mapChipField_ == nullptr) 
	{
		return;
	}

	const Vector3 currentCenter = worldTransform_.translation_;

	// X方向だけ移動させる
	Vector3 movedCenter = currentCenter;
	movedCenter.x += info.move.x;

	const std::array<Vector3, kNumCorner> currentCorners = GetCornerPositions(currentCenter);

	const std::array<Vector3, kNumCorner> movedCorners = GetCornerPositions(movedCenter);

	const Corner checkCorners[] = 
	{
		kLeftBottom,
		kLeftTop
	};

	bool isCollision = false;
	float rightmostBlockRight = 0.0f;

	for (Corner corner : checkCorners) 
	{
		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCorners[corner]);

		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCorners[corner]);

		if (currentIndex.xIndex == movedIndex.xIndex) 
		{
			continue;
		}

		if (mapChipField_->GetMapChipTypeByIndex(movedIndex.xIndex, movedIndex.yIndex) != MapChipType::kBlock)
		{
			continue;
		}

		const MapChipRect blockRect = mapChipField_->GetRectByIndex(movedIndex.xIndex, movedIndex.yIndex);

		if (!isCollision || blockRect.right > rightmostBlockRight)
		{
			rightmostBlockRight = blockRect.right;
		}

		isCollision = true;
	}

	if (!isCollision)
	{
		return;
	}

	// 敵の左端をブロック右端へ合わせる
	const float correctedCenterX = rightmostBlockRight + kWidth / 2.0f;

	info.move.x = correctedCenterX - worldTransform_.translation_.x;

	info.hitWall = true;
}

void Enemy::MapCollisionRight(CollisionMapInfo& info) 
{
	if (info.move.x <= 0.0f)
	{
		return;
	}

	if (mapChipField_ == nullptr) 
	{
		return;
	}

	const Vector3 currentCenter = worldTransform_.translation_;

	// X方向だけ移動させる
	Vector3 movedCenter = currentCenter;
	movedCenter.x += info.move.x;

	const std::array<Vector3, kNumCorner> currentCorners = GetCornerPositions(currentCenter);

	const std::array<Vector3, kNumCorner> movedCorners = GetCornerPositions(movedCenter);

	const Corner checkCorners[] = {kRightBottom, kRightTop};

	bool isCollision = false;
	float leftmostBlockLeft = 0.0f;

	for (Corner corner : checkCorners) 
	{
		// 境界上の座標を内側へ少しずらす
		Vector3 currentCheckPosition = currentCorners[corner];

		currentCheckPosition.x -= kCollisionEpsilon;

		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCheckPosition);

		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCorners[corner]);

		if (currentIndex.xIndex == movedIndex.xIndex) 
		{
			continue;
		}

		if (mapChipField_->GetMapChipTypeByIndex(movedIndex.xIndex, movedIndex.yIndex) != MapChipType::kBlock) 
		{
			continue;
		}

		const MapChipRect blockRect = mapChipField_->GetRectByIndex(movedIndex.xIndex, movedIndex.yIndex);

		if (!isCollision || blockRect.left < leftmostBlockLeft)
		{
			leftmostBlockLeft = blockRect.left;
		}

		isCollision = true;
	}

	if (!isCollision) 
	{
		return;
	}

	// 敵の右端をブロック左端へ合わせる
	const float correctedCenterX = leftmostBlockLeft - kWidth / 2.0f;

	info.move.x = correctedCenterX - worldTransform_.translation_.x;

	info.hitWall = true;
}

void Enemy::ApplyCollisionResult(const CollisionMapInfo& info) 
{
	// 補正後の移動量を反映
	worldTransform_.translation_.x += info.move.x;
	worldTransform_.translation_.y += info.move.y;
	worldTransform_.translation_.z += info.move.z;

	// 着地したら落下速度を止める
	if (info.landing && velocity_.y < 0.0f)
	{
		velocity_.y = 0.0f;
	}

    // 壁へ衝突したら横速度を止める
	if (info.hitWall) 
	{
		velocity_.x = 0.0f;
	}
}

void Enemy::UpdateOnGround(const CollisionMapInfo& info)
{
	onGround_ = info.landing; 
}

void Enemy::Draw() 
{
	if (isDead_ || model_ == nullptr || swordModel_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	// =========================
	// 本体描画用Transform
	// =========================
	drawWorldTransform_.scale_ = worldTransform_.scale_;
	drawWorldTransform_.rotation_ = worldTransform_.rotation_;
	drawWorldTransform_.translation_ = worldTransform_.translation_;

	// =========================
	// 剣描画用Transform
	// =========================
	drawSwordWorldTransform_.scale_ = swordWorldTransform_.scale_;
	drawSwordWorldTransform_.rotation_ = swordWorldTransform_.rotation_;
	drawSwordWorldTransform_.translation_ = swordWorldTransform_.translation_;

	// =========================
	// ガードされた時ののけ反り
	// =========================
	if (behavior_ == EnemyBehavior::kGuardStun) 
	{
		float progress = behaviorTimer_ / kGuardStunDuration;

		if (progress > 1.0f)
		{
			progress = 1.0f;
		}

		// 最初は大きくのけ反り、
		// 時間経過で元へ戻す
		const float recoilAngle = 0.35f * (1.0f - progress);

		if (direction_ == EnemyDirection::kRight)
		{
			drawWorldTransform_.rotation_.z += recoilAngle;
			drawSwordWorldTransform_.rotation_.z += recoilAngle;
		} 
		else 
		{
			drawWorldTransform_.rotation_.z -= recoilAngle;
			drawSwordWorldTransform_.rotation_.z -= recoilAngle;
		}
	}

	// 本体行列更新
	drawWorldTransform_.matWorld_ = MakeAffineMatrix(drawWorldTransform_.scale_, drawWorldTransform_.rotation_, drawWorldTransform_.translation_);

	drawWorldTransform_.TransferMatrix();

	// 剣行列更新
	drawSwordWorldTransform_.matWorld_ = MakeAffineMatrix(drawSwordWorldTransform_.scale_, drawSwordWorldTransform_.rotation_, drawSwordWorldTransform_.translation_);

	drawSwordWorldTransform_.TransferMatrix();

	// =========================
	// 本体描画
	// =========================
	if (damageFlashTimer_ > 0.0f && !isDeathAnimation_) 
	{
		model_->Draw(drawWorldTransform_, *camera_, &damageObjectColor_);
	}
	else 
	{
		model_->Draw(drawWorldTransform_, *camera_);
	}

	// 剣
	swordModel_->Draw(drawSwordWorldTransform_, *camera_);
}

Vector3 Enemy::GetWorldPosition() const
{
	return worldTransform_.translation_; 
}

void Enemy::OnHit(int damage)
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

	// HPを減らす
	hp_ -= damage;

	if (hp_ > 0) 
	{
		return;
	}

	hp_ = 0;

	// 死亡演出開始
	isDeathAnimation_ = true;
	deathAnimationTimer_ = 0.0f;

	velocity_ = {};
}

void Enemy::PushBack(float moveX) 
{
	// 死亡中・死亡演出中は押し返さない
	if (isDead_ || isDeathAnimation_)
	{
		return;
	}

	if (moveX == 0.0f)
	{
		return;
	}

	// ガードされたので攻撃を中断
	ChangeBehavior(EnemyBehavior::kGuardStun);

	// 横移動を停止
	velocity_.x = 0.0f;

	CollisionMapInfo info{};

	// 後退量
	info.move.x = moveX;

	// 壁を貫通しないように判定
	MapCollisionLeft(info);
	MapCollisionRight(info);

	ApplyCollisionResult(info);

	// 本体の行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 剣を追従
	swordWorldTransform_.scale_ = worldTransform_.scale_;
	swordWorldTransform_.rotation_ = worldTransform_.rotation_;
	swordWorldTransform_.translation_ = worldTransform_.translation_;

	swordWorldTransform_.matWorld_ = MakeAffineMatrix(swordWorldTransform_.scale_, swordWorldTransform_.rotation_, swordWorldTransform_.translation_);

	swordWorldTransform_.TransferMatrix();

}

bool Enemy::IsPlayerInAttackRange() const
{
	if (player_ == nullptr || player_->IsDead()) 
	{
		return false;
	}

	const Vector3 playerPosition = player_->GetWorldPosition();

	const float distanceX = std::abs(playerPosition.x - worldTransform_.translation_.x);

	const float distanceY = std::abs(playerPosition.y - worldTransform_.translation_.y);

	// 高さが大きく違う場合は攻撃しない
	if (distanceY > 1.0f) 
	{
		return false;
	}

	return distanceX <= kAttackStartDistance;
}

void Enemy::ChangeBehavior(EnemyBehavior behavior)
{
	behavior_ = behavior;
	behaviorTimer_ = 0.0f;

	// 新しい攻撃を開始するので命中記録をリセット
	if (behavior_ == EnemyBehavior::kAttack)
	{
		hasAttackHit_ = false;
	}

	// 歩行へ戻るときは剣の角度を元に戻す
	if (behavior_ == EnemyBehavior::kWalk)
	{
		swordAttackAngle_ = 0.0f;
	}
}

void Enemy::UpdateBehavior() 
{
	constexpr float kDeltaTime = 1.0f / 60.0f;

	behaviorTimer_ += kDeltaTime;

	switch (behavior_)
	{
	case EnemyBehavior::kWalk:
		swordAttackAngle_ = 0.0f;

		if (IsPlayerInAttackRange()) 
		{
			// 攻撃前にプレイヤーの方を向く
			const Vector3 playerPosition = player_->GetWorldPosition();

			if (playerPosition.x >= worldTransform_.translation_.x)
			{
				direction_ = EnemyDirection::kRight;
			} 
			else 
			{
				direction_ = EnemyDirection::kLeft;
			}

			ChangeBehavior(EnemyBehavior::kWindup);
		}
		break;

	case EnemyBehavior::kWindup:
	{
		// 0から1までの進行度
		float progress = behaviorTimer_ / kWindupDuration;

		if (progress > 1.0f) 
		{
			progress = 1.0f;
		}

		// 攻撃前に剣を後ろへ引く
		swordAttackAngle_ = kAttackSwordAngle * 0.5f * progress;

		if (behaviorTimer_ >= kWindupDuration)
		{
			ChangeBehavior(EnemyBehavior::kAttack);
		}
		break;
	}

	case EnemyBehavior::kAttack:
	{
		float progress = behaviorTimer_ / kAttackDuration;

		if (progress > 1.0f)
		{
			progress = 1.0f;
		}

		// 後方から前方へ剣を振る
		swordAttackAngle_ = kAttackSwordAngle * 0.5f - kAttackSwordAngle * 1.5f * progress;

		if (behaviorTimer_ >= kAttackDuration) 
		{
			ChangeBehavior(EnemyBehavior::kRecovery);
		}
		break;
	}

	case EnemyBehavior::kRecovery: 
	{
		float progress = behaviorTimer_ / kRecoveryDuration;

		if (progress > 1.0f)
		{
			progress = 1.0f;
		}

		// 振り終わった剣を元の角度へ戻す
		swordAttackAngle_ = -kAttackSwordAngle * (1.0f - progress);

		if (behaviorTimer_ >= kRecoveryDuration)
		{
			ChangeBehavior(EnemyBehavior::kWalk);
		}
		break;
	}

	case EnemyBehavior::kGuardStun:
	{
		if (behaviorTimer_ >= kGuardStunDuration)
		{
			ChangeBehavior(EnemyBehavior::kWalk);
		}

		break;
	}

	}
}

bool Enemy::IsAttackActive() const 
{
	// 攻撃行動中でなければ無効
	if (behavior_ != EnemyBehavior::kAttack) 
	{
		return false;
	}

	// すでに今回の攻撃が命中していたら無効
	if (hasAttackHit_)
	{
		return false;
	}

	// 攻撃時間に対する進行度
	const float progress = behaviorTimer_ / kAttackDuration;

	// 振り下ろしの中盤だけ有効
	return progress >= kAttackActiveStart && progress <= kAttackActiveEnd;
}

bool Enemy::IsPositionInAttackRange(const Vector3& position) const
{
	if (!IsAttackActive()) 
	{
		return false;
	}

	const Vector3 enemyPosition = worldTransform_.translation_;

	// 敵の正面に攻撃判定の中心を置く
	float hitBoxCenterX = enemyPosition.x;

	if (direction_ == EnemyDirection::kRight) 
	{
		hitBoxCenterX += kAttackHitBoxOffsetX;
	} 
	else
	{
		hitBoxCenterX -= kAttackHitBoxOffsetX;
	}

	const float distanceX = std::abs(position.x - hitBoxCenterX);

	const float distanceY = std::abs(position.y - enemyPosition.y);

	return distanceX <= kAttackHitBoxHalfWidth && distanceY <= kAttackHitBoxHalfHeight;
}

void Enemy::UpdateDeathAnimation() 
{
	constexpr float kDeltaTime = 1.0f / 60.0f;

	deathAnimationTimer_ += kDeltaTime;

	// Y軸方向へ回転
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

	// 剣も本体へ追従
	swordWorldTransform_.translation_ = worldTransform_.translation_;

	swordWorldTransform_.rotation_ = worldTransform_.rotation_;

	swordWorldTransform_.scale_ = worldTransform_.scale_;

	// 敵本体の行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 剣の行列更新
	swordWorldTransform_.matWorld_ = MakeAffineMatrix(swordWorldTransform_.scale_, swordWorldTransform_.rotation_, swordWorldTransform_.translation_);

	swordWorldTransform_.TransferMatrix();

	// 死亡演出終了
	if (deathAnimationTimer_ >= kDeathAnimationDuration) 
	{
		isDeathAnimation_ = false;
		isDead_ = true;
	}
}

bool Enemy::IsGroundAhead() const 
{
	if (mapChipField_ == nullptr)
	{
		return true;
	}

	Vector3 checkPosition = worldTransform_.translation_;

	// 敵の進行方向側の足元を見る
	if (direction_ == EnemyDirection::kRight) 
	{
		checkPosition.x += kWidth / 2.0f + 0.1f;
	}
	else 
	{
		checkPosition.x -= kWidth / 2.0f + 0.1f;
	}

	// 敵の足元より少し下
	checkPosition.y -= kHeight / 2.0f + 0.1f;

	const MapChipIndexSet index = mapChipField_->GetMapChipIndexSetByPosition(checkPosition);

	return mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex) == MapChipType::kBlock;
}