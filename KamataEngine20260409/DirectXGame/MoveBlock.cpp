#include "MoveBlock.h"
#include "MapChipField.h"
#include "MathUtility.h"
#include <cassert>
#include <numbers>

using namespace KamataEngine;

void MoveBlock::Initialize(Model* model, Camera* camera, const Vector3& position)
{
	assert(model);
	assert(camera);

	model_ = model;
	camera_ = camera;

	worldTransform_.Initialize();

	worldTransform_.translation_ = position;

	// 初期位置を保存
	startPosition_ = position;

	// 最初は右方向
	direction_ = 1;

	// 移動量を初期化
	moveAmount_ = {};

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();

}

void MoveBlock::Update() 
{
	const Vector3 previousPosition = worldTransform_.translation_;

	// 横移動
	if (moveDirection_ == MoveDirection::kHorizontal)
	{
		// 終点が初期位置より右にある場合
		if (endPositionX_ > startPosition_.x)
		{
			// 終点へ向かう
			if (direction_ > 0) 
			{
				worldTransform_.translation_.x += kMoveSpeed;

				if (worldTransform_.translation_.x >= endPositionX_)
				{
					worldTransform_.translation_.x = endPositionX_;
					direction_ = -1;
				}
			}
			// 初期位置へ戻る
			else 
			{
				worldTransform_.translation_.x -= kMoveSpeed;

				if (worldTransform_.translation_.x <= startPosition_.x)
				{
					worldTransform_.translation_.x = startPosition_.x;
					direction_ = 1;
				}
			}
		}
		// 終点が初期位置より左にある場合
		else
		{
			// 終点へ向かう
			if (direction_ > 0) 
			{
				worldTransform_.translation_.x -= kMoveSpeed;

				if (worldTransform_.translation_.x <= endPositionX_) 
				{
					worldTransform_.translation_.x = endPositionX_;
					direction_ = -1;
				}
			}
			// 初期位置へ戻る
			else
			{
				worldTransform_.translation_.x += kMoveSpeed;

				if (worldTransform_.translation_.x >= startPosition_.x)
				{
					worldTransform_.translation_.x = startPosition_.x;
					direction_ = 1;
				}
			}
		}
	}



	// 縦移動
	else 
	{
		// 終点が初期位置より上にある場合
		if (endPositionY_ > startPosition_.y)
		{
			// 終点へ向かう
			if (direction_ > 0)
			{
				worldTransform_.translation_.y += kMoveSpeed;

				if (worldTransform_.translation_.y >= endPositionY_) 
				{
					worldTransform_.translation_.y = endPositionY_;
					direction_ = -1;
				}
			}
			// 初期位置へ戻る
			else 
			{
				worldTransform_.translation_.y -= kMoveSpeed;

				if (worldTransform_.translation_.y <= startPosition_.y)
				{
					worldTransform_.translation_.y = startPosition_.y;
					direction_ = 1;
				}
			}
		}
		// 終点が初期位置より下にある場合
		else 
		{
			// 終点へ向かう
			if (direction_ > 0) 
			{
				worldTransform_.translation_.y -= kMoveSpeed;

				if (worldTransform_.translation_.y <= endPositionY_)
				{
					worldTransform_.translation_.y = endPositionY_;
					direction_ = -1;
				}
			}
			// 初期位置へ戻る
			else 
			{
				worldTransform_.translation_.y += kMoveSpeed;

				if (worldTransform_.translation_.y >= startPosition_.y) 
				{
					worldTransform_.translation_.y = startPosition_.y;
					direction_ = 1;
				}
			}
		}
	}

	moveAmount_ = 
	{
		worldTransform_.translation_.x - previousPosition.x,
		worldTransform_.translation_.y - previousPosition.y,
		worldTransform_.translation_.z - previousPosition.z
	};

	worldTransform_.matWorld_ = MakeAffineMatrix(worldTransform_.scale_, worldTransform_.rotation_, worldTransform_.translation_);

	worldTransform_.TransferMatrix();
}

void MoveBlock::Draw()
{
	if (model_ == nullptr || camera_ == nullptr) 
	{
		return;
	}

	model_->Draw(worldTransform_, *camera_);
}

Vector3 MoveBlock::GetWorldPosition() const 
{
	return worldTransform_.translation_; 
}

void MoveBlock::SearchRailEnd() 
{
	// マップチップが設定されていない場合
	if (mapChipField_ == nullptr)
	{
		endPositionX_ = startPosition_.x;
		endPositionY_ = startPosition_.y;
		return;
	}

	// G2のマップチップ番号を取得
	const MapChipIndexSet startIndex = mapChipField_->GetMapChipIndexSetByPosition(startPosition_);

	// 初期値
	endPositionX_ = startPosition_.x;
	endPositionY_ = startPosition_.y;

	// まず右側にR0があるか調べる
	if (mapChipField_->GetMapChipTypeByIndex(startIndex.xIndex + 1, startIndex.yIndex) == MapChipType::kBesideRail) 
	{
		moveDirection_ = MoveDirection::kHorizontal;

		int32_t currentX = startIndex.xIndex + 1;

		while (true)
		{
			if (mapChipField_->GetMapChipTypeByIndex(currentX, startIndex.yIndex) != MapChipType::kBesideRail)
			{
				break;
			}

			const Vector3 railPosition = mapChipField_->GetMapChipPositionByIndex(static_cast<uint32_t>(currentX), static_cast<uint32_t>(startIndex.yIndex));

			endPositionX_ = railPosition.x;

			++currentX;
		}

		// 横向きなので回転なし
		worldTransform_.rotation_.z = 0.0f;

		return;
	}

	// 左側にR0があるか調べる
	if (startIndex.xIndex > 0 && mapChipField_->GetMapChipTypeByIndex(startIndex.xIndex - 1, startIndex.yIndex) == MapChipType::kBesideRail)
	{
		moveDirection_ = MoveDirection::kHorizontal;

		int32_t currentX = startIndex.xIndex - 1;

		while (currentX >= 0)
		{
			if (mapChipField_->GetMapChipTypeByIndex(currentX, startIndex.yIndex) != MapChipType::kBesideRail)
			{
				break;
			}

			const Vector3 railPosition = mapChipField_->GetMapChipPositionByIndex(static_cast<uint32_t>(currentX), static_cast<uint32_t>(startIndex.yIndex));

			endPositionX_ = railPosition.x;

			--currentX;
		}

		// 横向きなので回転なし
		worldTransform_.rotation_.z = 0.0f;

		return;
	}

	// 次に下側にR1があるか調べる
	// CSVでは下へ行くほど yIndex が大きくなる
	if (mapChipField_->GetMapChipTypeByIndex(startIndex.xIndex, startIndex.yIndex + 1) == MapChipType::kVerticalRail)
	{
		moveDirection_ = MoveDirection::kVertical;

		int32_t currentY = startIndex.yIndex + 1;

		while (true) 
		{
			if (mapChipField_->GetMapChipTypeByIndex(startIndex.xIndex, currentY) != MapChipType::kVerticalRail) 
			{
				break;
			}

			const Vector3 railPosition = mapChipField_->GetMapChipPositionByIndex(static_cast<uint32_t>(startIndex.xIndex), static_cast<uint32_t>(currentY));

			endPositionY_ = railPosition.y;

			++currentY;
		}

		// 縦移動時は矢印を90度回転
		worldTransform_.rotation_.z = std::numbers::pi_v<float> / 2.0f;

		return;
	}

	// 上側にR1があるか調べる
	// CSVでは上へ行くほど yIndex が小さくなる
	if (startIndex.yIndex > 0 && mapChipField_->GetMapChipTypeByIndex(startIndex.xIndex, startIndex.yIndex - 1) == MapChipType::kVerticalRail) 
	{
		moveDirection_ = MoveDirection::kVertical;

		int32_t currentY = startIndex.yIndex - 1;

		while (currentY >= 0) 
		{
			if (mapChipField_->GetMapChipTypeByIndex(startIndex.xIndex, currentY) != MapChipType::kVerticalRail)
			{
				break;
			}

			const Vector3 railPosition = mapChipField_->GetMapChipPositionByIndex(static_cast<uint32_t>(startIndex.xIndex), static_cast<uint32_t>(currentY));

			endPositionY_ = railPosition.y;

			--currentY;
		}

		// 縦移動時は矢印を90度回転
		worldTransform_.rotation_.z = std::numbers::pi_v<float> / 2.0f;

		return;
	}
}