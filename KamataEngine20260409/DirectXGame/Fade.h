#pragma once
#include "KamataEngine.h"

using namespace KamataEngine;

class Fade
{
public:
	// フェードの状態
	enum class Status 
	{
		None,
		FadeIn,
		FadeOut,
	};

	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// フェード開始
	void Start(Status status, float duration);

	// 色を指定してフェード開始
	void Start(Status status, float duration, const Vector4& color);

	void Stop();

	// フェード終了判定
	bool IsFinished() const;

	// 現在フェード中か
	bool IsFading() const 
	{ 
		return status_ != Status::None; 
	}

private:
	Sprite* sprite_ = nullptr;

	Status status_ = Status::None;

	float duration_ = 0.0f;

	float counter_ = 0.0f;

	// フェード色
	Vector4 color_ = {0.0f, 0.0f, 0.0f, 1.0f};
};