#include "Spike.h"
#include "MathUtility.h"
#include <cassert>

using namespace KamataEngine;

void Spike::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Spike::Update()
{
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Spike::Draw()
{
	if (model_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}

Vector3 Spike::GetWorldPosition() const 
{
	return worldTransform_.translation_; 
}