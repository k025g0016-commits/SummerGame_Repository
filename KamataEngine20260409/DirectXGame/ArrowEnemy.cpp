#include "ArrowEnemy.h"
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

void ArrowEnemy::Initialize(Model* model, Model* arrowModel, Camera* camera, const Vector3& position, const Player* player)
{
	// nullptrチェック
	assert(model);
	assert(arrowModel);
	assert(camera);
	assert(player);

	model_ = model;
	arrowModel_ = arrowModel;
	camera_ = camera;
	player_ = player;

	// 必ずTransferMatrixより先に初期化する
	worldTransform_.Initialize();
	arrowWorldTransform_.Initialize();
	damageObjectColor_.Initialize();

	// 被弾時に使う赤色
	damageObjectColor_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});

	// 初期位置
	worldTransform_.translation_ = position;
	startPosition_ = position;

	// 速度を初期化
	velocity_ = {};

	// 最初は右へ移動
	direction_ = ArrowEnemyDirection::kRight;

	// 初期状態では未接地
	onGround_ = false;

	// モデルの大きさ
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	// 右向き
	worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;

	// 敵本体のワールド行列を更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

    // 弓を敵本体と同じ位置・向き・大きさにする
	arrowWorldTransform_.scale_ = worldTransform_.scale_;

	arrowWorldTransform_.rotation_ = worldTransform_.rotation_;

	arrowWorldTransform_.translation_ = worldTransform_.translation_;

	arrowWorldTransform_.matWorld_ = MakeAffineMatrix(arrowWorldTransform_.scale_, arrowWorldTransform_.rotation_, arrowWorldTransform_.translation_);

	arrowWorldTransform_.TransferMatrix();

	// HPなどの初期化
	hp_ = kMaxHP;
	isDead_ = false;
	isDeathAnimation_ = false;
	deathAnimationTimer_ = 0.0f;
	damageFlashTimer_ = 0.0f;
	stunTimer_ = 0.0f;

	behavior_ = ArrowEnemyBehavior::kWalk;
	behaviorTimer_ = 0.0f;
	hasAttackHit_ = false;

	shootRequested_ = false;

	previousPosition_ = position;

}

void ArrowEnemy::Update() 
{
	shootRequested_ = false;

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
		ReverseDirection();
	}

	// 初期位置を基準にした巡回範囲を確認
	// 巡回中に崖へ近づいたら引き返す
	if (behavior_ == ArrowEnemyBehavior::kWalk)
	{
		if (!IsGroundAhead())
		{
			ReverseDirection();
		}
	}

	// モデルの向きを更新
	if (direction_ == ArrowEnemyDirection::kRight)
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

	// 弓を敵本体に追従させる
	arrowWorldTransform_.scale_ = worldTransform_.scale_;
	arrowWorldTransform_.rotation_ = worldTransform_.rotation_;
	arrowWorldTransform_.translation_ = worldTransform_.translation_;

	// 弓のアニメーション
	switch (behavior_)
	{
	case ArrowEnemyBehavior::kAim:
	{
		// 徐々に弓を引く
		float progress = behaviorTimer_ / kAimDuration;

		if (progress > 1.0f) 
		{
			progress = 1.0f;
		}

		// 横へ伸ばす
		arrowWorldTransform_.scale_.z = 1.0f + 1.5f * progress;

		// 縦を縮める
		arrowWorldTransform_.scale_.y = 1.0f - 0.7f * progress;

		break;
	}

	case ArrowEnemyBehavior::kShoot:
	{
		// 最大まで引いた状態
		arrowWorldTransform_.scale_.x = 1.2f;
		arrowWorldTransform_.scale_.y = 0.8f;

		if (direction_ == ArrowEnemyDirection::kRight) 
		{
			arrowWorldTransform_.translation_.x -= 0.15f;
		}
		else 
		{
			arrowWorldTransform_.translation_.x += 0.15f;
		}

		break;
	}

	case ArrowEnemyBehavior::kRecovery:
	{
		float progress = behaviorTimer_ / kRecoveryDuration;

		if (progress > 1.0f) 
		{
			progress = 1.0f;
		}

		// 元の大きさへ戻す
		arrowWorldTransform_.scale_.x = 1.2f - 0.2f * progress;
		arrowWorldTransform_.scale_.y = 0.8f + 0.2f * progress;

		// 元の位置へ戻す
		if (direction_ == ArrowEnemyDirection::kRight) {
			arrowWorldTransform_.translation_.x -= 0.15f * (1.0f - progress);
		} else {
			arrowWorldTransform_.translation_.x += 0.15f * (1.0f - progress);
		}

		break;
	}

	default:
		break;
	}

	arrowWorldTransform_.matWorld_ = MakeAffineMatrix(arrowWorldTransform_.scale_, arrowWorldTransform_.rotation_, arrowWorldTransform_.translation_);

	arrowWorldTransform_.TransferMatrix();

}

void ArrowEnemy::Move()
{
	if (stunTimer_ > 0.0f)
	{
		velocity_.x = 0.0f;
		return;
	}

	if (behavior_ == ArrowEnemyBehavior::kAim || behavior_ == ArrowEnemyBehavior::kShoot || behavior_ == ArrowEnemyBehavior::kRecovery) 
	{
		velocity_.x = 0.0f;
		return;
	}

	if (direction_ == ArrowEnemyDirection::kRight)
	{
		velocity_.x = kWalkSpeed;
	}
	else 
	{
		velocity_.x = -kWalkSpeed;
	}
}

void ArrowEnemy::ReverseDirection()
{
	if (direction_ == ArrowEnemyDirection::kRight)
	{
		direction_ = ArrowEnemyDirection::kLeft;
	}
	else
	{
		direction_ = ArrowEnemyDirection::kRight;
	}
}

std::array<Vector3, ArrowEnemy::kNumCorner> ArrowEnemy::GetCornerPositions(const Vector3& center) const
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

void ArrowEnemy::MapCollision(CollisionMapInfo& info)
{
	// 現在の速度を移動量として設定
	info.move = velocity_;

	MapCollisionDown(info);
	MapCollisionLeft(info);
	MapCollisionRight(info);
}

void ArrowEnemy::MapCollisionDown(CollisionMapInfo& info)
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

void ArrowEnemy::MapCollisionLeft(CollisionMapInfo& info)
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

void ArrowEnemy::MapCollisionRight(CollisionMapInfo& info) 
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

void ArrowEnemy::ApplyCollisionResult(const CollisionMapInfo& info) 
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

void ArrowEnemy::UpdateOnGround(const CollisionMapInfo& info) { onGround_ = info.landing; }

void ArrowEnemy::Draw() 
{
	if (isDead_ || model_ == nullptr || arrowModel_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	// 敵本体
	if (damageFlashTimer_ > 0.0f && !isDeathAnimation_)
	{
		// 被弾中は赤色
		model_->Draw(worldTransform_, *camera_, &damageObjectColor_);
	}
	else
	{
		// 通常色
		model_->Draw(worldTransform_, *camera_);
	}

	// 武器
	arrowModel_->Draw(arrowWorldTransform_, *camera_);
}

Vector3 ArrowEnemy::GetWorldPosition() const
{ 
	return worldTransform_.translation_;
}

void ArrowEnemy::OnHit(int damage)
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

void ArrowEnemy::PushBack(float moveX) 
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
	stunTimer_ = kStunDuration;

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

	// 押し返された敵に武器を追従させる
	arrowWorldTransform_.scale_ = worldTransform_.scale_;
	arrowWorldTransform_.rotation_ = worldTransform_.rotation_;
	arrowWorldTransform_.translation_ = worldTransform_.translation_;

	arrowWorldTransform_.matWorld_ = MakeAffineMatrix(arrowWorldTransform_.scale_, arrowWorldTransform_.rotation_, arrowWorldTransform_.translation_);

	arrowWorldTransform_.TransferMatrix();
}

bool ArrowEnemy::IsPlayerInAttackRange() const 
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

void ArrowEnemy::ChangeBehavior(ArrowEnemyBehavior behavior) {
	behavior_ = behavior;
	behaviorTimer_ = 0.0f;

	// 新しい攻撃を開始するので命中記録をリセット
	if (behavior_ == ArrowEnemyBehavior::kShoot)
	{
		hasAttackHit_ = false;
	}

}

void ArrowEnemy::UpdateBehavior()
{
	constexpr float kDeltaTime = 1.0f / 60.0f;

	behaviorTimer_ += kDeltaTime;

	switch (behavior_)
	{
	case ArrowEnemyBehavior::kWalk:
		if (IsPlayerInAttackRange())
		{
			const Vector3 playerPosition = player_->GetWorldPosition();

			if (playerPosition.x >= worldTransform_.translation_.x)
			{
				direction_ = ArrowEnemyDirection::kRight;
			} 
			else
			{
				direction_ = ArrowEnemyDirection::kLeft;
			}

			ChangeBehavior(ArrowEnemyBehavior::kAim);
		}
		break;

	case ArrowEnemyBehavior::kAim:
		// 狙っている間もプレイヤー方向を向く
		if (player_ != nullptr && !player_->IsDead())
		{
			const Vector3 playerPosition = player_->GetWorldPosition();

			if (playerPosition.x >= worldTransform_.translation_.x) 
			{
				direction_ = ArrowEnemyDirection::kRight;
			} 
			else 
			{
				direction_ = ArrowEnemyDirection::kLeft;
			}
		}

		if (behaviorTimer_ >= kAimDuration)
		{
			ChangeBehavior(ArrowEnemyBehavior::kShoot);
		}
		break;

	case ArrowEnemyBehavior::kShoot:
		// このフレームに弾を発射する
		shootRequested_ = true;

		ChangeBehavior(ArrowEnemyBehavior::kRecovery);
		break;

	case ArrowEnemyBehavior::kRecovery:
		if (behaviorTimer_ >= kRecoveryDuration)
		{
			ChangeBehavior(ArrowEnemyBehavior::kWalk);
		}
		break;
	}
}

Vector3 ArrowEnemy::GetBulletSpawnPosition() const 
{
	Vector3 position = worldTransform_.translation_;

	// 敵の少し前方から発射
	if (direction_ == ArrowEnemyDirection::kRight)
	{
		position.x += 0.8f;
	} else {
		position.x -= 0.8f;
	}

	return position;
}

bool ArrowEnemy::IsGroundAhead() const 
{
	if (mapChipField_ == nullptr)
	{
		return true;
	}

	Vector3 checkPosition = worldTransform_.translation_;

	// 敵の進行方向側の足元を見る
	if (direction_ == ArrowEnemyDirection::kRight)
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

	const MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

	return mapChipType == MapChipType::kBlock || mapChipType == MapChipType::kSpikeBlock;
}

void ArrowEnemy::UpdateDeathAnimation()
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
	arrowWorldTransform_.translation_ = worldTransform_.translation_;
	arrowWorldTransform_.rotation_ = worldTransform_.rotation_;
	arrowWorldTransform_.scale_ = worldTransform_.scale_;

	// 本体の行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 武器の行列更新
	arrowWorldTransform_.matWorld_ = MakeAffineMatrix(arrowWorldTransform_.scale_, arrowWorldTransform_.rotation_, arrowWorldTransform_.translation_);

	arrowWorldTransform_.TransferMatrix();

	// 死亡演出終了
	if (deathAnimationTimer_ >= kDeathAnimationDuration)
	{
		isDeathAnimation_ = false;
		isDead_ = true;
	}
}

void ArrowEnemy::ResolveShutterDoorCollision(const Vector3& doorPosition)
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
	ReverseDirection();

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void ArrowEnemy::ResolveMoveBlockCollision(const Vector3& moveBlockPosition, const Vector3& moveBlockMoveAmount)
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

		ReverseDirection();
	}
	// 右側から接触
	else if (previousEnemyLeft >= previousBlockRight) 
	{
		worldTransform_.translation_.x = blockRight + enemyHalfWidth;

		velocity_.x = 0.0f;

		ReverseDirection();
	}

	// ワールド行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}