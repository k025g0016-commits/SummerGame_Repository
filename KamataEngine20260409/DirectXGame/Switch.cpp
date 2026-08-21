#include "Switch.h"
#include "MathUtility.h"
#include <cassert>

using namespace KamataEngine;

void Switch::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();
	objectColor_.Initialize();

	worldTransform_.translation_ = position;

	// 初期状態はOFF
	isOn_ = false;

	// OFF時は赤
	objectColor_.SetColor({1.0f, 0.0f, 0.0f, 1.0f});

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Switch::Update() 
{
	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void Switch::Draw()
{
	if (model_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_, &objectColor_);
}

Vector3 Switch::GetWorldPosition() const 
{
	return worldTransform_.translation_;
}

void Switch::OnSwitch()
{
	isOn_ = true;

	// ON時は緑
	objectColor_.SetColor({0.0f, 1.0f, 0.0f, 1.0f});
}