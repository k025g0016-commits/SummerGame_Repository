#include "CameraController.h"
#include "Player.h"
#include <algorithm>

using namespace KamataEngine;

void CameraController::Initialize(Camera* camera, const Player* player)
{
	camera_ = camera;
	player_ = player;

	if (camera_ == nullptr || player_ == nullptr) 
	{
		return;
	}

	// 初期位置ではプレイヤーの位置へ即座に合わせる
	const Vector3 playerPosition = player_->GetWorldPosition();

	camera_->translation_.x = playerPosition.x + targetOffset_.x;

	camera_->translation_.y = playerPosition.y + targetOffset_.y;

	camera_->translation_.z = playerPosition.z + targetOffset_.z;

	// 移動範囲内へ制限
	camera_->translation_.x = std::clamp(camera_->translation_.x, movableArea_.left, movableArea_.right);

	camera_->translation_.y = std::clamp(camera_->translation_.y, movableArea_.bottom, movableArea_.top);

	camera_->UpdateMatrix();
}

void CameraController::Update() 
{
	if (camera_ == nullptr || player_ == nullptr)
	{
		return;
	}

	const Vector3 playerPosition = player_->GetWorldPosition();

	// プレイヤーを基準にした目標カメラ位置
	Vector3 targetPosition{};

	targetPosition.x = playerPosition.x + targetOffset_.x;

	targetPosition.y = playerPosition.y + targetOffset_.y;

	targetPosition.z = playerPosition.z + targetOffset_.z;

	// 目標位置を移動可能範囲内へ制限
	targetPosition.x = std::clamp(targetPosition.x, movableArea_.left, movableArea_.right);

	targetPosition.y = std::clamp(targetPosition.y, movableArea_.bottom, movableArea_.top);

	// 現在位置から目標位置へ少しずつ近づける
	camera_->translation_.x += (targetPosition.x - camera_->translation_.x) * kInterpolationRate;

	camera_->translation_.y += (targetPosition.y - camera_->translation_.y) * kInterpolationRate;

	// Z座標は固定
	camera_->translation_.z = targetPosition.z;

	camera_->UpdateMatrix();
}

void CameraController::SetBossArea(float left, float right, float bottom, float top)
{
	constexpr float kViewHalfWidth = 10.0f;
	constexpr float kViewHalfHeight = 5.5f;

	// カメラ中心が移動できる範囲
	float cameraLeft = left + kViewHalfWidth;
	float cameraRight = right - kViewHalfWidth;

	float cameraBottom = bottom + kViewHalfHeight;
	float cameraTop = top - kViewHalfHeight;

	// 横幅が画面より狭い場合
	if (cameraLeft > cameraRight) 
	{
		// ボスエリア中央にカメラを固定
		const float centerX = (left + right) / 2.0f;

		cameraLeft = centerX;
		cameraRight = centerX;
	}

	// 縦幅が画面より狭い場合
	if (cameraBottom > cameraTop) 
	{
		// ボスエリア中央にカメラを固定
		const float centerY = (bottom + top) / 2.0f;

		cameraBottom = centerY;
		cameraTop = centerY;
	}

	movableArea_.left = cameraLeft;
	movableArea_.right = cameraRight;
	movableArea_.bottom = cameraBottom;
	movableArea_.top = cameraTop;
}