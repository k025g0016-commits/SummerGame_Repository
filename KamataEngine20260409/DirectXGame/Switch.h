#pragma once
#include "KamataEngine.h"

class Switch
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

	// スイッチを押す
	void OnSwitch();

	// ONか
	bool IsOn() const 
	{ 
		return isOn_; 
	}

private:
	// ワールド変換
	KamataEngine::WorldTransform worldTransform_;

	// モデル
	KamataEngine::Model* model_ = nullptr;

	// カメラ
	KamataEngine::Camera* camera_ = nullptr;

	// スイッチの色
	KamataEngine::ObjectColor objectColor_;

	// ONになっているか
	bool isOn_ = false;

};