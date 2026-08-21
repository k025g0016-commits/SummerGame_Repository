#include "BackGroundWall.h"
#include "MathUtility.h"
#include <numbers>

using namespace KamataEngine;

void BackGroundWall::Initialize(Model* model, const Camera* camera) 
{
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	// 確認用にプレイヤー付近へ配置
	worldTransform_.translation_ = {30.0f, -5.0f, 10.0f};
	worldTransform_.rotation_.y = std::numbers::pi_v<float>;

	// 確認しやすい大きさ
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void BackGroundWall::Update() {}

void BackGroundWall::Draw() 
{
	if (model_ == nullptr || camera_ == nullptr)
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}