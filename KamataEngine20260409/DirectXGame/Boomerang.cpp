#include "Boomerang.h"
#include "MathUtility.h"
#include "Player.h"
#include "MapChipField.h"
#include <numbers>

using namespace KamataEngine;

void Boomerang::Initialize(Model* model, const Camera* camera, Player* player) 
{
	model_ = model;
	camera_ = camera;
	player_ = player;

	worldTransform_.Initialize();

	// ringモデルの大きさに合わせて調整
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	phase_ = Phase::kHeld;

	UpdateHeld();

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Boomerang::Update() 
{
	switch (phase_)
	{
	case Phase::kHeld:
		UpdateHeld();
		break;

	case Phase::kOutbound:
		UpdateOutbound();
		break;

	case Phase::kReturn:
		UpdateReturn();
		break;
	}

	// 飛行中は回転する
	if (phase_ != Phase::kHeld)
	{
		worldTransform_.rotation_.z += kRotationSpeed;
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Boomerang::Draw() 
{
	if (model_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}

void Boomerang::Throw()
{
	// すでに飛んでいる場合は投げない
	if (phase_ != Phase::kHeld)
	{
		return;
	}

	// 現在の所持位置を投擲開始位置にする
	throwStartPosition_ = worldTransform_.translation_;

	if (player_->GetLRDirection() == Player::LRDirection::kRight) 
	{
		velocity_ = {kThrowSpeed, 0.0f, 0.0f};
	}
	else 
	{
		velocity_ = {-kThrowSpeed, 0.0f, 0.0f};
	}

	// 投擲時は反対向きにする
	worldTransform_.rotation_ = {0.0f, std::numbers::pi_v<float>, 0.0f};

	// 投擲時だけ少し奥へ移動
	worldTransform_.translation_.z += 0.5f;
	phase_ = Phase::kOutbound;

}

Vector3 Boomerang::GetWorldPosition() const 
{ 
	return {worldTransform_.matWorld_.m[3][0], worldTransform_.matWorld_.m[3][1], worldTransform_.matWorld_.m[3][2]};
}

void Boomerang::UpdateHeld() 
{ 
	// プレイヤーの手元に配置
	worldTransform_.translation_ = player_->GetShieldPosition();

	// ガード中は少し大きくする
	if (player_->IsGuarding()) 
	{
		worldTransform_.scale_ = {1.3f, 1.3f, 1.3f};
	}
	else 
	{
		worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};
	}

	// 所持中の盾の向き
	worldTransform_.rotation_ = {0.0f, 0.0f, 0.0f};

	if (player_->GetLRDirection() == Player::LRDirection::kRight)
	{
		// 右向き時
		worldTransform_.rotation_.y = std::numbers::pi_v<float> / 2.0f;
	}
	else 
	{
		// 左向き時
		worldTransform_.rotation_.y = -std::numbers::pi_v<float> / 2.0f;
	}
}

void Boomerang::UpdateOutbound() 
{
	// 移動前の位置を保存
	const Vector3 previousPosition = worldTransform_.translation_;

	// 行きの移動
	worldTransform_.translation_ = Add(worldTransform_.translation_, velocity_);

	// 行きのときだけ地形との衝突判定
	if (CheckMapCollision()) 
	{
		// 地形の中へ入り込まないように、
		// 移動前の位置へ戻す
		worldTransform_.translation_ = previousPosition;

		// 帰還開始
		StartReturn();

		return;
	}

	// 投擲開始位置からの距離
	const Vector3 difference = Subtract(worldTransform_.translation_, throwStartPosition_);

	// 最大距離に達したら帰還開始
	if (Length(difference) >= kMaxDistance) 
	{
		StartReturn();
	}
}

void Boomerang::UpdateReturn() 
{
	const Vector3 shieldPosition = player_->GetShieldPosition();

	const Vector3 difference = Subtract(shieldPosition, worldTransform_.translation_);

	const float distance = Length(difference);

	// 盾の所持位置へ近づいたら回収
	if (distance <= kCatchDistance)
	{
		phase_ = Phase::kHeld;
		velocity_ = {};
		UpdateHeld();
		return;
	}

	const Vector3 direction = Normalize(difference);

	worldTransform_.translation_ = Add(worldTransform_.translation_, Multiply(kReturnSpeed, direction));
}

void Boomerang::StartReturn()
{
	if (phase_ == Phase::kOutbound)
	{
		phase_ = Phase::kReturn;
	}
}

bool Boomerang::CheckMapCollision() const
{
	if (mapChipField_ == nullptr)
	{
		return false;
	}

	constexpr float kHalfWidth = 0.3f;
	constexpr float kHalfHeight = 0.3f;

	const Vector3 position = worldTransform_.translation_;

	// ブーメランの四隅
	const Vector3 checkPositions[] =
	{
	    {position.x - kHalfWidth, position.y - kHalfHeight, position.z},
	    {position.x + kHalfWidth, position.y - kHalfHeight, position.z},
	    {position.x - kHalfWidth, position.y + kHalfHeight, position.z},
	    {position.x + kHalfWidth, position.y + kHalfHeight, position.z},
	};

	for (const Vector3& checkPosition : checkPositions)
	{
		const MapChipIndexSet index = mapChipField_->GetMapChipIndexSetByPosition(checkPosition);

		const MapChipType mapChipType = mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex);

		// ブーメランの行きを遮る地形
		if (mapChipType == MapChipType::kBlock || mapChipType == MapChipType::kSpikeBlock || mapChipType == MapChipType::kShutterDoor)
		{
			return true;
		}
	}

	return false;
}