#include "Tutorial.h"
#include "MathUtility.h"
#include <numbers>

using namespace KamataEngine;

void Tutorial::Initialize(Model* plateModel, Model* tutorialModel, const Camera* camera, const Vector3& position)
{
	plateModel_ = plateModel;
	tutorialModel_ = tutorialModel;
	camera_ = camera;

	plateTransform_.Initialize();
	tutorialTransform_.Initialize();

	// 看板本体
	plateTransform_.translation_ = position;

	// 文字を少しだけカメラ側へ出す
	tutorialTransform_.translation_ = {position.x, position.y, position.z - 0.02f};

	// 必要なら向きを反転
	plateTransform_.rotation_.x = std::numbers::pi_v<float>/2.0f;

	tutorialTransform_.rotation_.x = std::numbers::pi_v<float>/2.0f;

	plateTransform_.rotation_.z = std::numbers::pi_v<float>;

	tutorialTransform_.rotation_.z = std::numbers::pi_v<float>;

	plateTransform_.matWorld_ = MakeAffineMatrix(plateTransform_.scale_, plateTransform_.rotation_, plateTransform_.translation_);

	tutorialTransform_.matWorld_ = MakeAffineMatrix(tutorialTransform_.scale_, tutorialTransform_.rotation_, tutorialTransform_.translation_);

	plateTransform_.TransferMatrix();
	tutorialTransform_.TransferMatrix();
}

void Tutorial::Update() {}

void Tutorial::Draw() 
{
	if (camera_ == nullptr) 
	{
		return;
	}

	if (plateModel_ != nullptr)
	{
		plateModel_->Draw(plateTransform_, *camera_);
	}

	if (tutorialModel_ != nullptr)
	{
		tutorialModel_->Draw(tutorialTransform_, *camera_);
	}
}