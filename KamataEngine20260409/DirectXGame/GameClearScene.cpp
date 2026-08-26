#include "GameClearScene.h"
#include "MathUtility.h"

using namespace KamataEngine;

void GameClearScene::Initialize() 
{
	returnTitleRequested_ = false;
	wasSpaceKeyPressed_ = false;

	spaceBlinkTimer_ = 0.0f;
	isSpaceWordVisible_ = true;

	// カメラ
	camera_.Initialize();
	camera_.translation_ = {0.0f, 0.0f, -25.0f};
	camera_.UpdateMatrix();

	// モデル読み込み
	if (backgroundModel_ == nullptr) 
	{
		backgroundModel_ = Model::CreateFromOBJ("GameClearBackground", true);
	}

	if (wordModel_ == nullptr)
	{
		wordModel_ = Model::CreateFromOBJ("GameClearWord", true);
	}

	if (spaceWordModel_ == nullptr)
	{
		spaceWordModel_ = Model::CreateFromOBJ("GameClearSPACEWord", true);
	}

	// Transform初期化
	backgroundTransform_.Initialize();
	wordTransform_.Initialize();
	spaceWordTransform_.Initialize();

	// カメラ側を向かせる
	backgroundTransform_.rotation_.y = std::numbers::pi_v<float>;
	wordTransform_.rotation_.y = std::numbers::pi_v<float>;
	spaceWordTransform_.rotation_.y = std::numbers::pi_v<float>;

	// 背景はゲームオーバーと同じ拡大率から開始
	backgroundTransform_.scale_ = {1.15f, 1.15f, 1.0f};

	// Zは文字を背景よりカメラ側へ
	backgroundTransform_.translation_ = {0.0f, 0.0f, 0.0f};
	wordTransform_.translation_ = {0.0f, 0.0f, -0.1f};
	spaceWordTransform_.translation_ = {0.0f, -2.0f, -0.2f};

	// 行列作成
	backgroundTransform_.matWorld_ = MakeAffineMatrix(backgroundTransform_.scale_, backgroundTransform_.rotation_, backgroundTransform_.translation_);

	wordTransform_.matWorld_ = MakeAffineMatrix(wordTransform_.scale_, wordTransform_.rotation_, wordTransform_.translation_);

	spaceWordTransform_.matWorld_ = MakeAffineMatrix(spaceWordTransform_.scale_, spaceWordTransform_.rotation_, spaceWordTransform_.translation_);

	backgroundTransform_.TransferMatrix();
	wordTransform_.TransferMatrix();
	spaceWordTransform_.TransferMatrix();

	// クリアBGM
	Audio* audio = Audio::GetInstance();

	clearBGMHandle_ = audio->LoadWave("BGM/Clear.wav");

	// ここでは再生しない
	isClearBGMStarted_ = false;

}

void GameClearScene::Update() 
{
	// クリアシーンが実際に更新された時にBGM開始
	if (!isClearBGMStarted_)
	{
		isClearBGMStarted_ = true;

		clearBGMVoiceHandle_ = Audio::GetInstance()->PlayWave(clearBGMHandle_, false, 0.5f);
	}

	Input* input = Input::GetInstance();

	const bool isSpaceKeyPressed = input->PushKey(DIK_SPACE);

	if (isSpaceKeyPressed && !wasSpaceKeyPressed_) 
	{
		// クリアBGMを停止
		if (clearBGMVoiceHandle_ != 0)
		{
			Audio::GetInstance()->StopWave(clearBGMVoiceHandle_);
			clearBGMVoiceHandle_ = 0;
		}

		returnTitleRequested_ = true;
	}

	wasSpaceKeyPressed_ = isSpaceKeyPressed;

	// SPACE案内の点滅
	spaceBlinkTimer_ += 1.0f / 60.0f;

	if (spaceBlinkTimer_ >= kSpaceBlinkInterval)
	{
		spaceBlinkTimer_ = 0.0f;
		isSpaceWordVisible_ = !isSpaceWordVisible_;
	}
}

void GameClearScene::Draw() 
{
	backgroundModel_->Draw(backgroundTransform_, camera_);

	wordModel_->Draw(wordTransform_, camera_);

	if (isSpaceWordVisible_) 
	{
		spaceWordModel_->Draw(spaceWordTransform_, camera_);
	}
}

GameClearScene::~GameClearScene()
{
	if (clearBGMVoiceHandle_ != 0) {
		Audio::GetInstance()->StopWave(clearBGMVoiceHandle_);
		clearBGMVoiceHandle_ = 0;
	}

	delete backgroundModel_;
	backgroundModel_ = nullptr;

	delete wordModel_;
	wordModel_ = nullptr;

	delete spaceWordModel_;
	spaceWordModel_ = nullptr;

}