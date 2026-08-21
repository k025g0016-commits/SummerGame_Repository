#include "ArrowBullet.h"
#include "MathUtility.h"
#include <cassert>
#include "MapChipField.h"
#include <numbers>

using namespace KamataEngine;

void ArrowBullet::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& velocity) 
{
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;
	velocity_ = velocity;

	// 弾の進行方向に合わせて向きを設定
	if (velocity_.x > 0.0f) 
	{
		worldTransform_.rotation_.y = std::numbers::pi_v<float>;
	}
	else
	{
		worldTransform_.rotation_.y = 0.0f;
	}

	lifeTimer_ = 0.0f;
	isDead_ = false;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	startPosition_ = position;

}

void ArrowBullet::Update() 
{
	if (isDead_)
	{
		return;
	}

	constexpr float kDeltaTime = 1.0f / 60.0f;

	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// 生存時間
	lifeTimer_ += kDeltaTime;

	if (lifeTimer_ >= kLifeTime) 
	{
		isDead_ = true;
		return;
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	if (mapChipField_ != nullptr) 
	{
		MapChipIndexSet index = mapChipField_->GetMapChipIndexSetByPosition(worldTransform_.translation_);

		if (mapChipField_->GetMapChipTypeByIndex(index.xIndex, index.yIndex) == MapChipType::kBlock)
		{
			isDead_ = true;
		}
	}

	if (Length(Subtract(worldTransform_.translation_, startPosition_)) > 12.0f)
	{
		isDead_ = true;
	}

}

void ArrowBullet::Draw()
{
	if (isDead_ || model_ == nullptr || camera_ == nullptr)
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}

Vector3 ArrowBullet::GetWorldPosition() const
{
	return worldTransform_.translation_; 
}