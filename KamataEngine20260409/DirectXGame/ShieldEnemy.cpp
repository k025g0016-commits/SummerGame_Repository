#include "ShieldEnemy.h"
#include "MapChipField.h"
#include "MathUtility.h"
#include "Player.h"
#include <array>
#include <cassert>
#include <cmath>
#include <numbers>

using namespace KamataEngine;

namespace 
{
constexpr float kCollisionEpsilon = 0.01f;
}

void ShieldEnemy::Initialize(KamataEngine::Model* model, KamataEngine::Model* shieldModel, KamataEngine::Camera* camera, const KamataEngine::Vector3& position, const Player* player)
{
	// nullptrチェック
	assert(model);
	assert(shieldModel);
	assert(camera);
	assert(player);

	model_ = model;
	shieldModel_ = shieldModel;
	camera_ = camera;
	player_ = player;

	// 必ずTransferMatrixより先に初期化する
	worldTransform_.Initialize();
	drawWorldTransform_.Initialize();
	shieldWorldTransform_.Initialize();
	damageObjectColor_.Initialize();

	// 被弾時に使う赤色
	damageObjectColor_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});

	// 初期位置
	worldTransform_.translation_ = position;
	startPosition_ = position;

	// 速度を初期化
	velocity_ = {};

	// 最初は右へ移動
	direction_ = ShieldEnemyDirection::kRight;

	// 初期状態では未接地
	onGround_ = false;

	// モデルの大きさ
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 右向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 敵本体のワールド行列を更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 盾を敵本体と同じ位置・向き・大きさにする
	shieldWorldTransform_.scale_ = worldTransform_.scale_;

	shieldWorldTransform_.rotation_ = worldTransform_.rotation_;

	shieldWorldTransform_.translation_ = worldTransform_.translation_;

	shieldWorldTransform_.matWorld_ = MakeAffineMatrix(shieldWorldTransform_.scale_, shieldWorldTransform_.rotation_, shieldWorldTransform_.translation_);

	shieldWorldTransform_.TransferMatrix();

	// HPなどの初期化
	hp_ = kMaxHP;
	isDead_ = false;
	isDeathAnimation_ = false;
	deathAnimationTimer_ = 0.0f;
	damageFlashTimer_ = 0.0f;
	stunTimer_ = 0.0f;

	behavior_ = ShieldEnemyBehavior::kWalk;
	behaviorTimer_ = 0.0f;
	shieldAttackAngle_ = 0.0f;

	previousPosition_ = position;

}

void ShieldEnemy::Update() 
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

	previousPosition_ = worldTransform_.translation_;

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

	// 壁に衝突したら進行方向を反転
	if (info.hitWall)
	{
		velocity_.x = 0.0f;

		if (behavior_ == ShieldEnemyBehavior::kCharge) 
		{
			ChangeBehavior(ShieldEnemyBehavior::kRecovery);
		}
		else 
		{
			ReverseDirection();
		}
	}

	// 通常巡回中は壁・崖の手前で反転
	if (behavior_ == ShieldEnemyBehavior::kWalk)
	{
		if (ShouldReverseWalkDirection())
		{
			ReverseDirection();
		}
	}

	// モデルの向きを更新
	if (direction_ == ShieldEnemyDirection::kRight)
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

	// 盾を敵本体に追従させる
	shieldWorldTransform_.scale_ = worldTransform_.scale_;
	shieldWorldTransform_.rotation_ = worldTransform_.rotation_;
	shieldWorldTransform_.translation_ = worldTransform_.translation_;

	// 盾の追加回転
	if (direction_ == ShieldEnemyDirection::kRight)
	{
		shieldWorldTransform_.rotation_.z += shieldAttackAngle_;
	}
	else
	{
		shieldWorldTransform_.rotation_.z -= shieldAttackAngle_;
	}

	shieldWorldTransform_.matWorld_ = MakeAffineMatrix(shieldWorldTransform_.scale_, shieldWorldTransform_.rotation_, shieldWorldTransform_.translation_);

	shieldWorldTransform_.TransferMatrix();
}

void ShieldEnemy::Move()
{
	if (stunTimer_ > 0.0f) 
	{
		velocity_.x = 0.0f;
		return;
	}

	// 溜め中・突進後の硬直中・ガード怯み中は停止
	if (behavior_ == ShieldEnemyBehavior::kWindup || behavior_ == ShieldEnemyBehavior::kRecovery || behavior_ == ShieldEnemyBehavior::kGuardStun)
	{
		velocity_.x = 0.0f;
		return;
	}

	float moveSpeed = kWalkSpeed;

	// 突進中
	if (behavior_ == ShieldEnemyBehavior::kCharge)
	{
		moveSpeed = kChargeSpeed;
	}

	if (direction_ == ShieldEnemyDirection::kRight) 
	{
		velocity_.x = moveSpeed;
	}
	else
	{
		velocity_.x = -moveSpeed;
	}

}

void ShieldEnemy::ReverseDirection() 
{
	// 突進中は向きを変えない
	// 溜め中・突進中は向きを変えない
	if (behavior_ == ShieldEnemyBehavior::kWindup || behavior_ == ShieldEnemyBehavior::kCharge)
	{
		return;
	}

	if (direction_ == ShieldEnemyDirection::kRight) 
	{
		direction_ = ShieldEnemyDirection::kLeft;
	}
	else 
	{
		direction_ = ShieldEnemyDirection::kRight;
	}
}

std::array<Vector3, ShieldEnemy::kNumCorner> ShieldEnemy::GetCornerPositions(const Vector3& center) const 
{
	std::array<Vector3, kNumCorner> positions;

	const float halfWidth = kWidth / 2.0f;
	const float halfHeight = kHeight / 2.0f;

	positions[kLeftBottom] = {center.x - halfWidth, center.y - halfHeight, center.z};

	positions[kRightBottom] = {center.x + halfWidth, center.y - halfHeight, center.z};

	positions[kLeftTop] = {center.x - halfWidth, center.y + halfHeight, center.z};

	positions[kRightTop] = {center.x + halfWidth, center.y + halfHeight, center.z};

	return positions;
}

void ShieldEnemy::MapCollision(CollisionMapInfo& info)
{
	// 現在の速度を移動量として設定
	info.move = velocity_;

	MapCollisionDown(info);
	MapCollisionLeft(info);
	MapCollisionRight(info);
}

void ShieldEnemy::MapCollisionDown(CollisionMapInfo& info) 
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

	const Corner checkCorners[] = {kLeftBottom, kRightBottom};

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

		const MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(movedIndex.xIndex, movedIndex.yIndex);

		if (mapChipType != MapChipType::kBlock && mapChipType != MapChipType::kSpikeBlock)
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

void ShieldEnemy::MapCollisionLeft(CollisionMapInfo& info) 
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

	const Corner checkCorners[] = {kLeftBottom, kLeftTop};

	bool isCollision = false;
	float rightmostBlockRight = 0.0f;

	for (Corner corner : checkCorners) 
	{
		Vector3 currentCheckPosition = currentCorners[corner];
		Vector3 movedCheckPosition = movedCorners[corner];

		// 足元の床を横壁として誤判定しないように、
		// 下側の判定点を少し上へずらす
		if (corner == kLeftBottom) 
		{
			currentCheckPosition.y += kCollisionEpsilon;
			movedCheckPosition.y += kCollisionEpsilon;
		}

		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCheckPosition);

		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCheckPosition);

		if (currentIndex.xIndex == movedIndex.xIndex)
		{
			continue;
		}

		const MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(movedIndex.xIndex, movedIndex.yIndex);

		if (mapChipType != MapChipType::kBlock && mapChipType != MapChipType::kSpikeBlock)
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

void ShieldEnemy::MapCollisionRight(CollisionMapInfo& info) 
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
		Vector3 currentCheckPosition = currentCorners[corner];
		Vector3 movedCheckPosition = movedCorners[corner];

		// 境界上の座標を内側へ少しずらす
		currentCheckPosition.x -= kCollisionEpsilon;

		// 足元の床を横壁として誤判定しないようにする
		if (corner == kRightBottom) 
		{
			currentCheckPosition.y += kCollisionEpsilon;
			movedCheckPosition.y += kCollisionEpsilon;
		}

		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCheckPosition);

		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCheckPosition);

		if (currentIndex.xIndex == movedIndex.xIndex) 
		{
			continue;
		}

		const MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(movedIndex.xIndex, movedIndex.yIndex);

		if (mapChipType != MapChipType::kBlock && mapChipType != MapChipType::kSpikeBlock) 
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

void ShieldEnemy::ApplyCollisionResult(const CollisionMapInfo& info)
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

void ShieldEnemy::UpdateOnGround(const CollisionMapInfo& info)
{
	onGround_ = info.landing;
}

void ShieldEnemy::Draw()
{
	if (isDead_ || model_ == nullptr || shieldModel_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	// 通常時は本体の変換をそのまま使う
	drawWorldTransform_.scale_ = worldTransform_.scale_;
	drawWorldTransform_.rotation_ = worldTransform_.rotation_;
	drawWorldTransform_.translation_ = worldTransform_.translation_;

    // 死亡演出中でなければ通常の行動モーションを反映
	if (!isDeathAnimation_) 
	{
		// 溜め中
		if (behavior_ == ShieldEnemyBehavior::kWindup)
		{
			// 前後を縮め、上下へ伸ばす
			drawWorldTransform_.scale_.z = 0.75f;
			drawWorldTransform_.scale_.y = 1.15f;
		}

		// 突進中
		else if (behavior_ == ShieldEnemyBehavior::kCharge)
		{
			// 前後へ伸ばし、上下を縮める
			drawWorldTransform_.scale_.z = 1.2f;
			drawWorldTransform_.scale_.y = 0.85f;
		}

		// ガードで攻撃を受けた後ののけ反り
		else if (behavior_ == ShieldEnemyBehavior::kGuardStun)
		{
			float progress = behaviorTimer_ / kGuardStunDuration;

			if (progress > 1.0f) 
			{
				progress = 1.0f;
			}

			const float recoilAngle = 0.35f * (1.0f - progress);

			if (direction_ == ShieldEnemyDirection::kRight) 
			{
				drawWorldTransform_.rotation_.z += recoilAngle;
				shieldWorldTransform_.rotation_.z += recoilAngle;
			}
			else 
			{
				drawWorldTransform_.rotation_.z -= recoilAngle;
				shieldWorldTransform_.rotation_.z -= recoilAngle;
			}
		}
	}

	drawWorldTransform_.matWorld_ = MakeAffineMatrix(drawWorldTransform_.scale_, drawWorldTransform_.rotation_, drawWorldTransform_.translation_);

	drawWorldTransform_.TransferMatrix();

	shieldWorldTransform_.matWorld_ = MakeAffineMatrix(shieldWorldTransform_.scale_, shieldWorldTransform_.rotation_, shieldWorldTransform_.translation_);

	shieldWorldTransform_.TransferMatrix();

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

	// 盾
	shieldModel_->Draw(shieldWorldTransform_, *camera_);
}

Vector3 ShieldEnemy::GetWorldPosition() const 
{ 
	return worldTransform_.translation_;
}

bool ShieldEnemy::IsPositionInFront(const Vector3& position) const 
{
	if (direction_ == ShieldEnemyDirection::kRight) 
	{
		return position.x >= worldTransform_.translation_.x;
	} 
	else 
	{
		return position.x <= worldTransform_.translation_.x;
	}
}

void ShieldEnemy::OnHit(int damage) 
{
	// すでに死亡している場合は何もしない
	if (isDead_ || isDeathAnimation_) 
	{
		return;
	}

	// 0以下のダメージは受け付けない
	if (damage <= 0) 
	{
		return;
	}

	// 被弾時の赤表示開始
	damageFlashTimer_ = kDamageFlashDuration;

	// HPを減らす
	hp_ -= damage;

	// HPが残っている場合は死亡しない
	if (hp_ > 0)
	{
		return;
	}

	// HPがマイナスにならないように補正
	hp_ = 0;

	// 死亡状態にする
	isDeathAnimation_ = true;
	deathAnimationTimer_ = 0.0f;

	// 移動を停止
	velocity_ = {};
}

void ShieldEnemy::PushBack(float moveX) 
{
	// 死亡している場合は押し返さない
	if (isDead_ || isDeathAnimation_) 
	{
		return;
	}

	// 移動量が0なら何もしない
	if (moveX == 0.0f) 
	{
		return;
	}

	// 押し返されたので怯ませる
	ChangeBehavior(ShieldEnemyBehavior::kGuardStun);

	CollisionMapInfo info{};

	// 横方向の押し返し量を設定
	info.move.x = moveX;

	// 押し返す方向にある壁との衝突判定
	MapCollisionLeft(info);
	MapCollisionRight(info);

	// 補正された移動量を反映
	ApplyCollisionResult(info);

	// ワールド行列を更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 押し返された敵に盾を追従させる
	shieldWorldTransform_.scale_ = worldTransform_.scale_;
	shieldWorldTransform_.rotation_ = worldTransform_.rotation_;
	shieldWorldTransform_.translation_ = worldTransform_.translation_;

	shieldWorldTransform_.matWorld_ = MakeAffineMatrix(shieldWorldTransform_.scale_, shieldWorldTransform_.rotation_, shieldWorldTransform_.translation_);

	shieldWorldTransform_.TransferMatrix();
}

void ShieldEnemy::ChangeBehavior(ShieldEnemyBehavior behavior) 
{
	behavior_ = behavior;
	behaviorTimer_ = 0.0f;

}

void ShieldEnemy::StopCharge()
{
	if (behavior_ == ShieldEnemyBehavior::kCharge)
	{
		ChangeBehavior(ShieldEnemyBehavior::kRecovery);
	}
}

void ShieldEnemy::UpdateBehavior()
{
	constexpr float kDeltaTime = 1.0f / 60.0f;

	behaviorTimer_ += kDeltaTime;

	switch (behavior_)
	{
	case ShieldEnemyBehavior::kWalk:
	{
		if (IsPlayerInChargeRange()) 
		{
			// すぐ突進せず、まず溜め状態へ
			ChangeBehavior(ShieldEnemyBehavior::kWindup);
		}

		break;
	}

	case ShieldEnemyBehavior::kWindup: 
	{
		// 一定時間溜めたら突進開始
		if (behaviorTimer_ >= kWindupDuration) 
		{
			// 実際に突進を始める位置を保存
			chargeStartPosition_ = worldTransform_.translation_;

			ChangeBehavior(ShieldEnemyBehavior::kCharge);
		}

		break;
	}

	case ShieldEnemyBehavior::kCharge: 
	{
		const Vector3 difference = Subtract(worldTransform_.translation_, chargeStartPosition_);

		const float distance = Length(difference);

		// 最大距離まで進んだら突進終了
		if (distance >= kChargeMaxDistance) 
		{
			ChangeBehavior(ShieldEnemyBehavior::kRecovery);
		}

		break;
	}

	case ShieldEnemyBehavior::kRecovery:
	{
		if (behaviorTimer_ >= kRecoveryDuration)
		{
			ChangeBehavior(ShieldEnemyBehavior::kWalk);
		}

		break;
	}

	case ShieldEnemyBehavior::kGuardStun:
	{
		if (behaviorTimer_ >= kGuardStunDuration)
		{
			ChangeBehavior(ShieldEnemyBehavior::kWalk);
		}

		break;
	}
	}

}

bool ShieldEnemy::IsPlayerInChargeRange() const
{
	if (player_ == nullptr || player_->IsDead())
	{
		return false;
	}

	const Vector3 playerPosition = player_->GetWorldPosition();
	const Vector3 enemyPosition = worldTransform_.translation_;

	const float distanceX = std::abs(playerPosition.x - enemyPosition.x);

	const float distanceY = std::abs(playerPosition.y - enemyPosition.y);

	// 高さが大きく違う場合は反応しない
	if (distanceY > 1.0f) 
	{
		return false;
	}

	// 距離外なら反応しない
	if (distanceX > kChargeStartDistance) 
	{
		return false;
	}

	// 右向きならプレイヤーが右側にいる必要がある
	if (direction_ == ShieldEnemyDirection::kRight) 
	{
		return playerPosition.x >= enemyPosition.x;
	}

	// 左向きならプレイヤーが左側にいる必要がある
	return playerPosition.x <= enemyPosition.x;
}

void ShieldEnemy::OnGuard()
{
	if (isDead_ || isDeathAnimation_)
	{
		return;
	}

	// 移動を停止
	velocity_.x = 0.0f;

	// 溜め中・突進中であっても強制的に中断して怯み状態へ
	ChangeBehavior(ShieldEnemyBehavior::kGuardStun);

	// 敵が向いている方向とは逆へ後退
	float moveX = 0.0f;

	if (direction_ == ShieldEnemyDirection::kRight)
	{
		moveX = -0.6f;
	}
	else
	{
		moveX = 0.6f;
	}

	CollisionMapInfo info{};
	info.move.x = moveX;

	MapCollisionLeft(info);
	MapCollisionRight(info);

	ApplyCollisionResult(info);

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 盾も追従
	shieldWorldTransform_.scale_ = worldTransform_.scale_;
	shieldWorldTransform_.rotation_ = worldTransform_.rotation_;
	shieldWorldTransform_.translation_ = worldTransform_.translation_;

	shieldWorldTransform_.matWorld_ = MakeAffineMatrix(shieldWorldTransform_.scale_, shieldWorldTransform_.rotation_, shieldWorldTransform_.translation_);

	shieldWorldTransform_.TransferMatrix();
}

void ShieldEnemy::UpdateDeathAnimation() 
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

	// 武器も追従
	shieldWorldTransform_.translation_ = worldTransform_.translation_;
	shieldWorldTransform_.rotation_ = worldTransform_.rotation_;
	shieldWorldTransform_.scale_ = worldTransform_.scale_;

	// 本体の行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 武器の行列更新
	shieldWorldTransform_.matWorld_ = MakeAffineMatrix(shieldWorldTransform_.scale_, shieldWorldTransform_.rotation_, shieldWorldTransform_.translation_);

	shieldWorldTransform_.TransferMatrix();

	// 死亡演出終了
	if (deathAnimationTimer_ >= kDeathAnimationDuration) 
	{
		isDeathAnimation_ = false;
		isDead_ = true;
	}
}

bool ShieldEnemy::ShouldReverseWalkDirection() const
{
	if (mapChipField_ == nullptr) 
	{
		return false;
	}

	const Vector3 enemyPosition = worldTransform_.translation_;

	const float halfWidth = kWidth / 2.0f;
	const float halfHeight = kHeight / 2.0f;

	// 敵の進行方向側を少し先まで調べる
	constexpr float kCheckForward = 0.1f;

	float checkX = enemyPosition.x;

	if (direction_ == ShieldEnemyDirection::kRight) 
	{
		checkX += halfWidth + kCheckForward;
	} 
	else 
	{
		checkX -= halfWidth + kCheckForward;
	}

	// 壁チェック
	// 敵の中央付近の高さを調べる
	const Vector3 wallCheckPosition = {checkX, enemyPosition.y, enemyPosition.z};

	const MapChipIndexSet wallIndex = mapChipField_->GetMapChipIndexSetByPosition(wallCheckPosition);

	const MapChipType wallType = mapChipField_->GetMapChipTypeByIndex(wallIndex.xIndex, wallIndex.yIndex);

	if (wallType == MapChipType::kBlock || wallType == MapChipType::kSpikeBlock) 
	{
		return true;
	}

	// 崖チェック
	// 進行方向側の足元より少し下を調べる
	const Vector3 groundCheckPosition = {checkX, enemyPosition.y - halfHeight - kCheckForward, enemyPosition.z};

	const MapChipIndexSet groundIndex = mapChipField_->GetMapChipIndexSetByPosition(groundCheckPosition);

	const MapChipType groundType = mapChipField_->GetMapChipTypeByIndex(groundIndex.xIndex, groundIndex.yIndex);

	if (groundType != MapChipType::kBlock && groundType != MapChipType::kSpikeBlock) 
	{
		return true;
	}

	return false;
}

void ShieldEnemy::ResolveShutterDoorCollision(const Vector3& doorPosition)
{
	if (isDead_ || isDeathAnimation_)
	{
		return;
	}

	constexpr float kDoorHalfWidth = 0.5f;
	constexpr float kDoorHalfHeight = 0.5f;

	const float enemyHalfWidth = kWidth / 2.0f;
	const float enemyHalfHeight = kHeight / 2.0f;

	const float enemyLeft = worldTransform_.translation_.x - enemyHalfWidth;

	const float enemyRight = worldTransform_.translation_.x + enemyHalfWidth;

	const float enemyBottom = worldTransform_.translation_.y - enemyHalfHeight;

	const float enemyTop = worldTransform_.translation_.y + enemyHalfHeight;

	const float doorLeft = doorPosition.x - kDoorHalfWidth;

	const float doorRight = doorPosition.x + kDoorHalfWidth;

	const float doorBottom = doorPosition.y - kDoorHalfHeight;

	const float doorTop = doorPosition.y + kDoorHalfHeight;

	// 重なっていなければ何もしない
	if (enemyRight <= doorLeft || enemyLeft >= doorRight || enemyTop <= doorBottom || enemyBottom >= doorTop) 
	{
		return;
	}

	// 敵が扉の左側にいる
	if (worldTransform_.translation_.x < doorPosition.x) 
	{
		worldTransform_.translation_.x = doorLeft - enemyHalfWidth;
	}
	else 
	{
		worldTransform_.translation_.x = doorRight + enemyHalfWidth;
	}

	// 横移動を止める
	velocity_.x = 0.0f;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void ShieldEnemy::ResolveMoveBlockCollision(const Vector3& moveBlockPosition, const Vector3& moveBlockMoveAmount)
{
	if (isDead_ || isDeathAnimation_) 
	{
		return;
	}

	constexpr float kMoveBlockWidth = 1.0f;
	constexpr float kMoveBlockHeight = 1.0f;

	const float enemyHalfWidth = kWidth / 2.0f;
	const float enemyHalfHeight = kHeight / 2.0f;

	const float moveBlockHalfWidth = kMoveBlockWidth / 2.0f;

	const float moveBlockHalfHeight = kMoveBlockHeight / 2.0f;

	// 現在の敵矩形
	const float enemyLeft = worldTransform_.translation_.x - enemyHalfWidth;

	const float enemyRight = worldTransform_.translation_.x + enemyHalfWidth;

	const float enemyBottom = worldTransform_.translation_.y - enemyHalfHeight;

	const float enemyTop = worldTransform_.translation_.y + enemyHalfHeight;

	// 現在のMoveBlock矩形
	const float blockLeft = moveBlockPosition.x - moveBlockHalfWidth;

	const float blockRight = moveBlockPosition.x + moveBlockHalfWidth;

	const float blockBottom = moveBlockPosition.y - moveBlockHalfHeight;

	const float blockTop = moveBlockPosition.y + moveBlockHalfHeight;

	// 重なっていなければ何もしない
	if (enemyRight <= blockLeft || enemyLeft >= blockRight || enemyTop <= blockBottom || enemyBottom >= blockTop) 
	{
		return;
	}

	// MoveBlockの移動前位置
	const Vector3 previousMoveBlockPosition = {moveBlockPosition.x - moveBlockMoveAmount.x, moveBlockPosition.y - moveBlockMoveAmount.y, moveBlockPosition.z - moveBlockMoveAmount.z};

	// 敵の前フレーム矩形
	const float previousEnemyLeft = previousPosition_.x - enemyHalfWidth;

	const float previousEnemyRight = previousPosition_.x + enemyHalfWidth;

	const float previousEnemyBottom = previousPosition_.y - enemyHalfHeight;

	const float previousEnemyTop = previousPosition_.y + enemyHalfHeight;

	// MoveBlockの前フレーム矩形
	const float previousBlockLeft = previousMoveBlockPosition.x - moveBlockHalfWidth;

	const float previousBlockRight = previousMoveBlockPosition.x + moveBlockHalfWidth;

	const float previousBlockBottom = previousMoveBlockPosition.y - moveBlockHalfHeight;

	const float previousBlockTop = previousMoveBlockPosition.y + moveBlockHalfHeight;

	// 上から接触
	if (previousEnemyBottom >= previousBlockTop)
	{
		worldTransform_.translation_.y = blockTop + enemyHalfHeight;

		if (velocity_.y < 0.0f)
		{
			velocity_.y = 0.0f;
		}

		onGround_ = true;
	}
	// 下から接触
	else if (previousEnemyTop <= previousBlockBottom) 
	{
		worldTransform_.translation_.y = blockBottom - enemyHalfHeight;

		if (velocity_.y > 0.0f)
		{
			velocity_.y = 0.0f;
		}
	}
	// 左側から接触
	else if (previousEnemyRight <= previousBlockLeft)
	{
		worldTransform_.translation_.x = blockLeft - enemyHalfWidth;

		velocity_.x = 0.0f;

		if (behavior_ == ShieldEnemyBehavior::kCharge)
		{
			ChangeBehavior(ShieldEnemyBehavior::kRecovery);
		} 
		else 
		{
			ReverseDirection();
		}
	}
	// 右側から接触
	else if (previousEnemyLeft >= previousBlockRight)
	{
		worldTransform_.translation_.x = blockRight + enemyHalfWidth;

		velocity_.x = 0.0f;

		if (behavior_ == ShieldEnemyBehavior::kCharge) 
		{
			ChangeBehavior(ShieldEnemyBehavior::kRecovery);
		}
		else 
		{
			ReverseDirection();
		}
	}

	// ワールド行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 盾も敵本体へ追従
	shieldWorldTransform_.scale_ = worldTransform_.scale_;

	shieldWorldTransform_.rotation_ = worldTransform_.rotation_;

	shieldWorldTransform_.translation_ = worldTransform_.translation_;

	shieldWorldTransform_.matWorld_ = MakeAffineMatrix(shieldWorldTransform_.scale_, shieldWorldTransform_.rotation_, shieldWorldTransform_.translation_);

	shieldWorldTransform_.TransferMatrix();
}