#include "RingEffect.h"
#include "MathUtility.h"
#include <numbers>

using namespace KamataEngine;

void RingEffect::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	worldTransform_.scale_ = {kStartScale, kStartScale, kStartScale};

	// カメラ側へ向ける
	worldTransform_.rotation_.y = std::numbers::pi_v<float>;

	timer_ = 0.0f;
	isFinished_ = false;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void RingEffect::Update()
{
	if (isFinished_) 
	{
		return;
	}

	constexpr float kDeltaTime = 1.0f / 60.0f;

	timer_ += kDeltaTime;

	float progress = timer_ / kDuration;

	if (progress > 1.0f) 
	{
		progress = 1.0f;
	}

	// 小さい状態から大きくする
	const float scale = kStartScale + (kEndScale - kStartScale) * progress;

	worldTransform_.scale_ = {scale, scale, scale};

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	if (timer_ >= kDuration)
	{
		isFinished_ = true;
	}
}

void RingEffect::Draw()
{
	if (isFinished_ || model_ == nullptr || camera_ == nullptr)
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}