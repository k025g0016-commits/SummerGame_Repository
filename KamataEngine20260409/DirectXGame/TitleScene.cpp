#include "TitleScene.h"

using namespace KamataEngine;

void TitleScene::Initialize() 
{
	startRequested_ = false;
	wasSpaceKeyPressed_ = false;
}

void TitleScene::Update()
{
	Input* input = Input::GetInstance();

	// 現在のSpaceキー入力
	const bool isEnterKeyPressed = input->PushKey(DIK_SPACE);

	// Spaceを押した瞬間にゲーム開始
	if (isEnterKeyPressed && !wasSpaceKeyPressed_) 
	{
		startRequested_ = true;
	}

	// 次フレーム用
	wasSpaceKeyPressed_ = isEnterKeyPressed;
}

void TitleScene::Draw() 
{
	// 現在は何も描画しない
	// 後からタイトル画像を表示する
}