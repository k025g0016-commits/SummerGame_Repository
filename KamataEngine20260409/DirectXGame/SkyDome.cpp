#include "SkyDome.h"
#include "MathUtility.h"

using namespace KamataEngine;

void SkyDome::Initialize(Model* model, const Camera* camera)
{
	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	// 天球を大きくする
	worldTransform_.scale_ = {1.0f, 1.0f, 1.0f};

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void SkyDome::Update()
{
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void SkyDome::Draw()
{
	if (model_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}