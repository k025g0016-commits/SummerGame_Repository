#include "Player.h"
#include "MapChipField.h"
#include "MathUtility.h"
#include <numbers>
#include <array>

using namespace KamataEngine;

namespace
{
    constexpr float kCollisionEpsilon = 0.01f;
}

void Player::Initialize(Model* model, const Camera* camera, const Vector3& position)
{
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	drawWorldTransform_.Initialize();

	worldTransform_.translation_ = position;

	velocity_ = {};
	knockBackVelocityX_ = 0.0f;
	onGround_ = false;
	isGuarding_ = false;
	wasJumpKeyPressed_ = false;

	// HPを最大値にする
	hp_ = kMaxHP;

	// 生存状態にする
	isDead_ = false;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 更新前位置も初期位置にする
	previousPosition_ = position;

}

void Player::Update() 
{
	// 死亡している場合は操作・更新しない
	if (isDead_)
	{
		return;
	}

	// 今フレームの移動前位置を保存
	previousPosition_ = worldTransform_.translation_;

	// 盾構え状態を更新
	UpdateGuard();

	// 移動入力
	InputMove();

	// ノックバック速度を加算
	velocity_.x += knockBackVelocityX_;

	// ノックバックを徐々に減速
	if (knockBackVelocityX_ > 0.0f) 
	{
		knockBackVelocityX_ -= kKnockBackDeceleration;

		if (knockBackVelocityX_ < 0.0f)
		{
			knockBackVelocityX_ = 0.0f;
		}
	}
	else if (knockBackVelocityX_ < 0.0f)
	{
		knockBackVelocityX_ += kKnockBackDeceleration;

		if (knockBackVelocityX_ > 0.0f) 
		{
			knockBackVelocityX_ = 0.0f;
		}
	}

	// 重力
	velocity_.y -= kGravity;

	// 落下速度制限
	if (velocity_.y < -kLimitFallSpeed)
	{
		velocity_.y = -kLimitFallSpeed;
	}

	CollisionMapInfo info{};

	MapCollision(info);
	ApplyCollisionResult(info);
	UpdateOnGround(info);

	// モデルの向きを更新
	if (lrDirection_ == LRDirection::kRight)
	{
		worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	}
	else 
	{
		worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Player::InputMove() 
{
	Input* input = Input::GetInstance();

	// 毎フレーム、横速度を初期化
	velocity_.x = 0.0f;

	// 通常時の移動速度
	float moveSpeed = kMoveSpeed;

	// 盾を構えている間は移動速度を下げる
	if (isGuarding_)
	{
		moveSpeed *= kGuardMoveRate;
	}

	const bool moveLeft = input->PushKey(DIK_A) || input->PushKey(DIK_LEFT);

	const bool moveRight = input->PushKey(DIK_D) || input->PushKey(DIK_RIGHT);

	// 左入力のみ
	if (moveLeft && !moveRight) 
	{
		velocity_.x = -moveSpeed;

		// 構え中は向きを変更しない
		if (!isGuarding_) 
		{
			lrDirection_ = LRDirection::kLeft;
		}
	}
	// 右入力のみ
	else if (moveRight && !moveLeft)
	{
		velocity_.x = moveSpeed;

		// 構え中は向きを変更しない
		if (!isGuarding_) 
		{
			lrDirection_ = LRDirection::kRight;
		}
	}

	// 現在のSpaceキー入力
	const bool isJumpKeyPressed = input->PushKey(DIK_SPACE);

	// 押した瞬間かつ接地中ならジャンプ
	if (isJumpKeyPressed && !wasJumpKeyPressed_ && onGround_) 
	{
		velocity_.y = kJumpSpeed;
		onGround_ = false;
	}

	// 次フレーム用に保存
	wasJumpKeyPressed_ = isJumpKeyPressed;
}

void Player::Draw() 
{
	if (isDead_ || model_ == nullptr || camera_ == nullptr)
	{
		return;
	}

	drawWorldTransform_.scale_ = worldTransform_.scale_;
	drawWorldTransform_.rotation_ = worldTransform_.rotation_;
	drawWorldTransform_.translation_ = worldTransform_.translation_;

	if (isGuarding_)
	{
		drawWorldTransform_.scale_ = {1.05f, 0.8f, 1.0f};

		// 足元が浮かないよう、見た目だけ少し下げる
		drawWorldTransform_.translation_.y -= 0.08f;
	}

	drawWorldTransform_.matWorld_ = MakeAffineMatrix(drawWorldTransform_.scale_, drawWorldTransform_.rotation_, drawWorldTransform_.translation_);

	drawWorldTransform_.TransferMatrix();

	model_->Draw(drawWorldTransform_, *camera_);
}

Vector3 Player::GetWorldPosition() const
{
	return
	{
		worldTransform_.matWorld_.m[3][0],
		worldTransform_.matWorld_.m[3][1], 
		worldTransform_.matWorld_.m[3][2]
	};
}

Vector3 Player::GetShieldPosition() const 
{
	const Vector3 playerPosition = GetWorldPosition();

	if (lrDirection_ == LRDirection::kRight)
	{
		return
		{
			playerPosition.x + 0.7f,
			playerPosition.y,
			playerPosition.z
		};
	} 
	else 
	{
		return 
		{
			playerPosition.x - 0.7f,
			playerPosition.y,
			playerPosition.z
		};
	}
}

std::array<Vector3, Player::kNumCorner> Player::GetCornerPositions(const Vector3& center) const 
{
	std::array<Vector3, kNumCorner> positions;

	const float halfWidth = kWidth / 2.0f;
	const float halfHeight = kHeight / 2.0f;

	// 左下
	positions[kLeftBottom] = 
	{
		center.x - halfWidth,
		center.y - halfHeight,
		center.z
	};

	// 右下
	positions[kRightBottom] =
	{
		center.x + halfWidth,
		center.y - halfHeight,
		center.z
	};

	// 左上
	positions[kLeftTop] = 
	{
		center.x - halfWidth,
		center.y + halfHeight,
		center.z
	};

	// 右上
	positions[kRightTop] =
	{
		center.x + halfWidth,
		center.y + halfHeight,
		center.z
	};

	return positions;
}

void Player::MapCollision(CollisionMapInfo& info)
{
	// 現在の速度をそのまま移動量として設定
	info.move = velocity_;

	// 各方向の衝突判定
	MapCollisionUp(info);
	MapCollisionDown(info);
	MapCollisionLeft(info);
	MapCollisionRight(info);
}

void Player::MapCollisionDown(CollisionMapInfo& info)
{
	// 下方向へ移動していない場合は判定しない
	if (info.move.y >= 0.0f) 
	{
		return;
	}

	// マップチップフィールドが設定されていない場合
	if (mapChipField_ == nullptr) 
	{
		return;
	}

	// 移動前の中心座標
	const Vector3 currentCenter = worldTransform_.translation_;

	// 下方向の判定なのでY方向だけ移動させる
	Vector3 movedCenter = currentCenter;
	movedCenter.y += info.move.y;

	// 移動前と移動後の四隅
	const std::array<Vector3, kNumCorner> currentCorners = GetCornerPositions(currentCenter);

	const std::array<Vector3, kNumCorner> movedCorners = GetCornerPositions(movedCenter);

	// 下側の2点を調べる
	const Corner checkCorners[] = {kLeftBottom, kRightBottom};

	bool isCollision = false;

	// 衝突した床のうち、最も高い位置
	float highestBlockTop = 0.0f;

	for (Corner corner : checkCorners) 
	{
		// ブロック側面を床として誤判定しないように、
		// 左右の角をプレイヤー内側へ少しずらす
		Vector3 currentCheckPosition = currentCorners[corner];
		Vector3 movedCheckPosition = movedCorners[corner];

		if (corner == kLeftBottom) 
		{
			currentCheckPosition.x += kCollisionEpsilon;
			movedCheckPosition.x += kCollisionEpsilon;
		}
		else if (corner == kRightBottom)
		{
			currentCheckPosition.x -= kCollisionEpsilon;
			movedCheckPosition.x -= kCollisionEpsilon;
		}

		// 移動前のマップチップ番号
		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCheckPosition);

		// 移動後のマップチップ番号
		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCheckPosition);

		// Y方向のマップチップ境界を越えていなければ判定しない
		if (currentIndex.yIndex == movedIndex.yIndex) 
		{
			continue;
		}

		// 移動後の位置がブロックでなければ判定しない
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

	// 床へ衝突していなければ終了
	if (!isCollision)
	{
		return;
	}

	// プレイヤーの下端がブロック上端に接する位置へ補正
	const float correctedCenterY = highestBlockTop + kHeight / 2.0f;

	info.move.y = correctedCenterY - worldTransform_.translation_.y;

	info.landing = true;
}

void Player::MapCollisionUp(CollisionMapInfo& info)
{
	// 上方向へ移動していない場合は判定しない
	if (info.move.y <= 0.0f)
	{
		return;
	}

	// マップチップフィールドが設定されていない場合
	if (mapChipField_ == nullptr)
	{
		return;
	}

	// 移動前の中心座標
	const Vector3 currentCenter = worldTransform_.translation_;

// 上方向の判定なのでY方向だけ移動させる
	Vector3 movedCenter = currentCenter;
	movedCenter.y += info.move.y;

	// 移動前と移動後の四隅
	const std::array<Vector3, kNumCorner> currentCorners = GetCornerPositions(currentCenter);

	const std::array<Vector3, kNumCorner> movedCorners = GetCornerPositions(movedCenter);

	// 上側の2点を調べる
	const Corner checkCorners[] = {kLeftTop, kRightTop};

	bool isCollision = false;

	// 衝突した天井のうち、最も低い位置
	float lowestBlockBottom = 0.0f;

	for (Corner corner : checkCorners)
	{
		// ブロック側面を天井として誤判定しないように、
		// 左右の角をプレイヤー内側へ少しずらす
		Vector3 currentCheckPosition = currentCorners[corner];
		Vector3 movedCheckPosition = movedCorners[corner];

		if (corner == kLeftTop)
		{
			currentCheckPosition.x += kCollisionEpsilon;
			movedCheckPosition.x += kCollisionEpsilon;
		}
		else if (corner == kRightTop) 
		{
			currentCheckPosition.x -= kCollisionEpsilon;
			movedCheckPosition.x -= kCollisionEpsilon;
		}

		// 移動前のマップチップ番号
		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCheckPosition);

		// 移動後のマップチップ番号
		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCheckPosition);

		// Y方向のマップチップ境界を越えていなければ判定しない
		if (currentIndex.yIndex == movedIndex.yIndex)
		{
			continue;
		}

		// 移動後の位置がブロックでなければ判定しない
		if (mapChipField_->GetMapChipTypeByIndex(movedIndex.xIndex, movedIndex.yIndex) != MapChipType::kBlock) 
		{
			continue;
		}

		const MapChipRect blockRect = mapChipField_->GetRectByIndex(movedIndex.xIndex, movedIndex.yIndex);

		if (!isCollision || blockRect.bottom < lowestBlockBottom) 
		{
			lowestBlockBottom = blockRect.bottom;
		}

		isCollision = true;
	}

	// 天井へ衝突していなければ終了
	if (!isCollision)
	{
		return;
	}

	// プレイヤーの上端がブロック下端に接する位置へ補正
	const float correctedCenterY = lowestBlockBottom - kHeight / 2.0f;

	info.move.y = correctedCenterY - worldTransform_.translation_.y;

	info.ceiling = true;
}

void Player::MapCollisionLeft(CollisionMapInfo& info)
{
	// 左方向へ移動していない場合は判定しない
	if (info.move.x >= 0.0f)
	{
		return;
	}

	// マップチップフィールドが設定されていない場合
	if (mapChipField_ == nullptr) 
	{
		return;
	}

	// 移動前の中心座標
	const Vector3 currentCenter = worldTransform_.translation_;

	// 左方向の判定なのでX方向だけ移動させる
	Vector3 movedCenter = currentCenter;
	movedCenter.x += info.move.x;

	// 移動前と移動後の四隅
	const std::array<Vector3, kNumCorner> currentCorners = GetCornerPositions(currentCenter);

	const std::array<Vector3, kNumCorner> movedCorners = GetCornerPositions(movedCenter);

	// 左側の2点を調べる
	const Corner checkCorners[] = {kLeftBottom, kLeftTop};

	bool isCollision = false;

	// 衝突した壁のうち、最も右側の位置
	float rightmostBlockRight = 0.0f;

	for (Corner corner : checkCorners)
	{
		// 移動前のマップチップ番号
		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCorners[corner]);

		// 移動後のマップチップ番号
		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCorners[corner]);

		// X方向のマップチップ境界を越えていなければ判定しない
		if (currentIndex.xIndex == movedIndex.xIndex)
		{
			continue;
		}

		// 移動後の位置がブロックでなければ判定しない
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

	// 左壁へ衝突していなければ終了
	if (!isCollision) 
	{
		return;
	}

	// プレイヤーの左端がブロック右端に接する位置へ補正
	const float correctedCenterX = rightmostBlockRight + kWidth / 2.0f;

	info.move.x = correctedCenterX - worldTransform_.translation_.x;

	info.hitWall = true;
}

void Player::MapCollisionRight(CollisionMapInfo& info) 
{
	// 右方向へ移動していない場合は判定しない
	if (info.move.x <= 0.0f)
	{
		return;
	}

	// マップチップフィールドが設定されていない場合
	if (mapChipField_ == nullptr)
	{
		return;
	}

	// 移動前の中心座標
	const Vector3 currentCenter = worldTransform_.translation_;

	// 右方向の判定なのでX方向だけ移動させる
	Vector3 movedCenter = currentCenter;
	movedCenter.x += info.move.x;

	// 移動前と移動後の四隅
	const std::array<Vector3, kNumCorner> currentCorners = GetCornerPositions(currentCenter);

	const std::array<Vector3, kNumCorner> movedCorners = GetCornerPositions(movedCenter);

	// 右側の2点を調べる
	const Corner checkCorners[] = {kRightBottom, kRightTop};

	bool isCollision = false;

	// 衝突した壁のうち、最も左側の位置
	float leftmostBlockLeft = 0.0f;

	for (Corner corner : checkCorners) 
	{
		// 境界上の座標が右側のマスとして扱われないように、
		// 移動前の右端をわずかにプレイヤー内側へずらす
		Vector3 currentCheckPosition = currentCorners[corner];
		currentCheckPosition.x -= kCollisionEpsilon;

		const MapChipIndexSet currentIndex = mapChipField_->GetMapChipIndexSetByPosition(currentCheckPosition);

		const MapChipIndexSet movedIndex = mapChipField_->GetMapChipIndexSetByPosition(movedCorners[corner]);

		// X方向のマップチップ境界を越えていなければ判定しない
		if (currentIndex.xIndex == movedIndex.xIndex) 
		{
			continue;
		}

		// 移動後の位置がブロックでなければ判定しない
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

	// 右壁へ衝突していなければ終了
	if (!isCollision) 
	{
		return;
	}

	// プレイヤーの右端がブロック左端に接する位置へ補正
	const float correctedCenterX = leftmostBlockLeft - kWidth / 2.0f;

	info.move.x = correctedCenterX - worldTransform_.translation_.x;

	info.hitWall = true;
}

void Player::ApplyCollisionResult(const CollisionMapInfo& info) 
{
	// 補正後の移動量を反映
	worldTransform_.translation_.x += info.move.x;
	worldTransform_.translation_.y += info.move.y;
	worldTransform_.translation_.z += info.move.z;

	// 天井へ衝突したら上向き速度を止める
	if (info.ceiling)
	{
		if (velocity_.y > 0.0f) 
		{
			velocity_.y = 0.0f;
		}
	}

	// 着地したら下向き速度を止める
	if (info.landing)
	{
		if (velocity_.y < 0.0f) 
		{
			velocity_.y = 0.0f;
		}
	}

	// 壁へ衝突したら横速度を止める
	if (info.hitWall) 
	{
		velocity_.x = 0.0f;
		knockBackVelocityX_ = 0.0f;
	}
}

void Player::UpdateOnGround(const CollisionMapInfo& info)
{
	if (info.landing) 
	{
		onGround_ = true;
	}
	else
	{
		onGround_ = false;
	}
}

void Player::UpdateGuard() 
{
	Input* input = Input::GetInstance();

	// Kキーを押している間、盾を構える
	isGuarding_ = input->PushKey(DIK_LSHIFT);
}

void Player::OnHit(int damage) 
{
	// すでに死亡している場合は何もしない
	if (isDead_) 
	{
		return;
	}

	// 0以下のダメージは受け付けない
	if (damage <= 0) 
	{
		return;
	}

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
	isDead_ = true;

	// 移動を停止
	velocity_ = {};

	// ガード状態も解除
	isGuarding_ = false;
}

void Player::PushBack(float moveX) 
{
	// 0なら何もしない
	if (moveX == 0.0f) 
	{
		return;
	}

	// ノックバック速度として保存
	knockBackVelocityX_ = moveX;
}

void Player::MoveBy(const Vector3& move)
{
	if (isDead_) 
	{
		return;
	}

	worldTransform_.translation_.x += move.x;
	worldTransform_.translation_.y += move.y;
	worldTransform_.translation_.z += move.z;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Player::LandOnMoveBlock(float moveBlockTop)
{
	if (isDead_)
	{
		return;
	}

	// プレイヤーの下端がMoveBlockの上面に合うように補正
	worldTransform_.translation_.y = moveBlockTop + kHeight / 2.0f;

	// 落下速度を止める
	if (velocity_.y < 0.0f) 
	{
		velocity_.y = 0.0f;
	}

	// 接地状態にする
	onGround_ = true;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Player::ResolveMoveBlockCollision(const Vector3& moveBlockPosition, const Vector3& moveBlockMoveAmount)
{
	if (isDead_)
	{
		return;
	}

	constexpr float kMoveBlockWidth = 1.0f;
	constexpr float kMoveBlockHeight = 1.0f;

	const float playerHalfWidth = kWidth / 2.0f;
	const float playerHalfHeight = kHeight / 2.0f;

	const float moveBlockHalfWidth = kMoveBlockWidth / 2.0f;
	const float moveBlockHalfHeight = kMoveBlockHeight / 2.0f;

	// 現在のプレイヤー矩形
	const float playerLeft = worldTransform_.translation_.x - playerHalfWidth;

	const float playerRight = worldTransform_.translation_.x + playerHalfWidth;

	const float playerBottom = worldTransform_.translation_.y - playerHalfHeight;

	const float playerTop = worldTransform_.translation_.y + playerHalfHeight;

	// 現在のMoveBlock矩形
	const float blockLeft = moveBlockPosition.x - moveBlockHalfWidth;

	const float blockRight = moveBlockPosition.x + moveBlockHalfWidth;

	const float blockBottom = moveBlockPosition.y - moveBlockHalfHeight;

	const float blockTop = moveBlockPosition.y + moveBlockHalfHeight;

	// 現在重なっていなければ何もしない
	if (playerRight <= blockLeft || playerLeft >= blockRight || playerTop <= blockBottom || playerBottom >= blockTop) 
	{
		return;
	}

	// MoveBlockの移動前位置
	const Vector3 previousMoveBlockPosition = {moveBlockPosition.x - moveBlockMoveAmount.x, moveBlockPosition.y - moveBlockMoveAmount.y, moveBlockPosition.z - moveBlockMoveAmount.z};

	const float previousPlayerLeft = previousPosition_.x - playerHalfWidth;

	const float previousPlayerRight = previousPosition_.x + playerHalfWidth;

	const float previousPlayerBottom = previousPosition_.y - playerHalfHeight;

	const float previousPlayerTop = previousPosition_.y + playerHalfHeight;

	const float previousBlockLeft = previousMoveBlockPosition.x - moveBlockHalfWidth;

	const float previousBlockRight = previousMoveBlockPosition.x + moveBlockHalfWidth;

	const float previousBlockBottom = previousMoveBlockPosition.y - moveBlockHalfHeight;

	const float previousBlockTop = previousMoveBlockPosition.y + moveBlockHalfHeight;

	// 上から接触
	if (previousPlayerBottom >= previousBlockTop)
	{
		worldTransform_.translation_.y = blockTop + playerHalfHeight;

		if (velocity_.y < 0.0f) 
		{
			velocity_.y = 0.0f;
		}

		onGround_ = true;
	}
	// 下から接触
	else if (previousPlayerTop <= previousBlockBottom) 
	{
		worldTransform_.translation_.y = blockBottom - playerHalfHeight;

		if (velocity_.y > 0.0f)
		{
			velocity_.y = 0.0f;
		}
	}
	// 左側から接触
	else if (previousPlayerRight <= previousBlockLeft)
	{
		worldTransform_.translation_.x = blockLeft - playerHalfWidth;

		velocity_.x = 0.0f;
		knockBackVelocityX_ = 0.0f;
	}
	// 右側から接触
	else if (previousPlayerLeft >= previousBlockRight)
	{
		worldTransform_.translation_.x = blockRight + playerHalfWidth;

		velocity_.x = 0.0f;
		knockBackVelocityX_ = 0.0f;
	}

	// ワールド行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}