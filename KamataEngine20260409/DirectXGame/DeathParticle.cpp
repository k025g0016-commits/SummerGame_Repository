#include "DeathParticle.h"
#include "MathUtility.h"

#include <algorithm>
#include <cmath>

using namespace KamataEngine;

void DeathParticle::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	model_ = model;
	camera_ = camera;

	objectColor_.Initialize();

	color_ = {1.0f, 1.0f, 1.0f, 1.0f};

	objectColor_.SetColor(color_);

	for (WorldTransform& worldTransform : worldTransforms_)
	{
		worldTransform.Initialize();

		worldTransform.translation_ = position;

		// 最初のサイズ
		worldTransform.scale_ = {0.5f, 0.5f, 0.5f};

		// カメラ側へ向ける
		worldTransform.rotation_.y = 3.1415926535f;

		worldTransform.matWorld_ = MakeAffineMatrix(worldTransform.scale_, worldTransform.rotation_, worldTransform.translation_);

		worldTransform.TransferMatrix();
	}

	timer_ = 0.0f;
	isFinished_ = false;
}

void DeathParticle::Update() 
{
	if (isFinished_)
	{
		return;
	}

	constexpr float kDeltaTime = 1.0f / 60.0f;

	timer_ += kDeltaTime;

	// 時間経過で透明にする
	color_.w = std::clamp(1.0f - timer_ / kDuration, 0.0f, 1.0f);

	objectColor_.SetColor(color_);

	for (uint32_t i = 0; i < kNumParticles; ++i)
	{
		const float angle = kAngleUnit * static_cast<float>(i);

		const Vector3 velocity = {std::cos(angle) * kSpeed, std::sin(angle) * kSpeed, 0.0f};

		worldTransforms_[i].translation_.x += velocity.x;
		worldTransforms_[i].translation_.y += velocity.y;
		worldTransforms_[i].translation_.z += velocity.z;

		worldTransforms_[i].matWorld_ = MakeAffineMatrix(worldTransforms_[i].scale_, worldTransforms_[i].rotation_, worldTransforms_[i].translation_);

		worldTransforms_[i].TransferMatrix();
	}

	if (timer_ >= kDuration)
	{
		timer_ = kDuration;
		isFinished_ = true;
	}
}

void DeathParticle::Draw()
{
	if (isFinished_ || model_ == nullptr || camera_ == nullptr)
	{
		return;
	}

	for (WorldTransform& worldTransform : worldTransforms_)
	{
		model_->Draw(worldTransform, *camera_, &objectColor_);
	}
}