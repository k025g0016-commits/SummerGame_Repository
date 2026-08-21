#include "ShutterDoor.h"
#include "MathUtility.h"
#include <cassert>

using namespace KamataEngine;

void ShutterDoor::Initialize(Model* model, Camera* camera, const Vector3& position) 
{
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	// マップチップで指定された位置
	worldTransform_.translation_ = position;

	// 初期位置を保存
	startPosition_ = position;

	// 初期状態では閉じている
	isOpen_ = false;

	// 開いたときの目標位置
	openTargetY_ = startPosition_.y + kOpenDistance;

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

	isHidden_ = false;

}

void ShutterDoor::Update()
{
	// 開いている場合は上へ移動
	if (isOpen_) 
	{
		worldTransform_.translation_.y += kOpenSpeed;

		// 目標位置を超えないようにする
		if (worldTransform_.translation_.y >= openTargetY_)
		{
			worldTransform_.translation_.y = openTargetY_;

			// 完全に開いたので非表示にする
			isHidden_ = true;
		}
	}

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void ShutterDoor::Draw() 
{
	if (isHidden_ || model_ == nullptr || camera_ == nullptr)
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}

Vector3 ShutterDoor::GetWorldPosition() const
{
	return worldTransform_.translation_;
}

void ShutterDoor::Open() 
{
	isOpen_ = true;
}