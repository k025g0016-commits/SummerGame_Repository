#include "SparkParticle.h"
#include "MathUtility.h"
#include <numbers>

using namespace KamataEngine;

void SparkParticle::Initialize(Model* model, Camera* camera, const Vector3& position, const Vector3& velocity)
{
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;
	worldTransform_.scale_ = {kStartScale, kStartScale, kStartScale};
	worldTransform_.rotation_.y = std::numbers::pi_v<float>;
	velocity_ = velocity;

	timer_ = 0.0f;
	isFinished_ = false;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void SparkParticle::Update() 
{
	if (isFinished_)
	{
		return;
	}

	constexpr float kDeltaTime = 1.0f / 60.0f;

	timer_ += kDeltaTime;

	// 移動
	worldTransform_.translation_.x += velocity_.x;
	worldTransform_.translation_.y += velocity_.y;
	worldTransform_.translation_.z += velocity_.z;

	// 徐々に小さくする
	float progress = timer_ / kLifeTime;

	if (progress > 1.0f)
	{
		progress = 1.0f;
	}

	const float scale = kStartScale + (kEndScale - kStartScale) * progress;

	worldTransform_.scale_ = {scale, scale, scale};

	// 行列更新
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	// 寿命終了
	if (timer_ >= kLifeTime) 
	{
		isFinished_ = true;
	}
}

void SparkParticle::Draw()
{
	if (isFinished_ || model_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}