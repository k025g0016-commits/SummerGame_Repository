#pragma once
#include "KamataEngine.h"

class MapChipField;
class MoveBlock
{
public:
	// 初期化
	void Initialize(KamataEngine::Model* model, KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ワールド座標を取得
	KamataEngine::Vector3 GetWorldPosition() const;

	// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField) 
	{
		mapChipField_ = mapChipField;

		// マップチップ設定後にレール終端を調べる
		SearchRailEnd();
	}

	// 今フレームの移動量を取得
	KamataEngine::Vector3 GetMoveAmount() const
	{
		return moveAmount_; 
	}

	enum class MoveDirection
	{ 
		kHorizontal,
		kVertical
	};

private:
	// ワールド変換
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// マップチップ
	MapChipField* mapChipField_ = nullptr;

	// 初期位置
	KamataEngine::Vector3 startPosition_ = {};

	// 移動方向
	int direction_ = 1;

	// 移動速度
	static inline const float kMoveSpeed = 0.03f;

	// 右端位置
	float endPositionX_ = 0.0f;

	// 右側のレール終端を調べる
	void SearchRailEnd();

	// 今フレームの移動量
	KamataEngine::Vector3 moveAmount_ = {};

	// 移動方向
	MoveDirection moveDirection_ = MoveDirection::kHorizontal;

	// 上端位置
	float endPositionY_ = 0.0f;

};