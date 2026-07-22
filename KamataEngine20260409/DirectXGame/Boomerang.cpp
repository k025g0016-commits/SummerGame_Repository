#include "Boomerang.h"
#include "MathUtility.h"
#include "Player.h"

using namespace KamataEngine;

void Boomerang::Initialize(Model* model, const Camera* camera, Player* player) 
{
	model_ = model;
	camera_ = camera;
	player_ = player;

	worldTransform_.Initialize();

	// ringモデルの大きさに合わせて調整
	worldTransform_.scale_ = {0.5f, 0.5f, 0.5f};

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
	} else {
		velocity_ = {-kThrowSpeed, 0.0f, 0.0f};
	}

	phase_ = Phase::kOutbound;
}

Vector3 Boomerang::GetWorldPosition() const 
{ 
	return {worldTransform_.matWorld_.m[3][0], worldTransform_.matWorld_.m[3][1], worldTransform_.matWorld_.m[3][2]};
}

void Boomerang::UpdateHeld() 
{ 
	worldTransform_.translation_ = player_->GetShieldPosition(); 
}

void Boomerang::UpdateOutbound() 
{
	worldTransform_.translation_ = Add(worldTransform_.translation_, velocity_);

	const Vector3 difference = Subtract(worldTransform_.translation_, throwStartPosition_);

	// 最大距離に達したら帰還開始
	if (Length(difference) >= kMaxDistance) 
	{
		phase_ = Phase::kReturn;
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