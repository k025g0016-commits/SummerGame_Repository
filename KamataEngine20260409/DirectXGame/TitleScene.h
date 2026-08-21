#pragma once
#include "KamataEngine.h"

class TitleScene 
{
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// ゲーム開始要求が出ているか
	bool IsStartRequested() const
	{
		return startRequested_;
	}

private:
	// ゲーム開始要求
	bool startRequested_ = false;

	// 前フレームでSpaceキーが押されていたか
	bool wasSpaceKeyPressed_ = false;
};