#include "GameOverScene.h"
#include "MathUtility.h"

using namespace KamataEngine;

void GameOverScene::Initialize() 
{
	// タイトルへ戻る要求を初期化
	returnTitleRequested_ = false;

	// Spaceキーの前フレーム状態を初期化
	wasSpaceKeyPressed_ = false;

	// カメラ
	camera_.Initialize();

	// 原点付近のモデルを見るため、
	// カメラを手前へ移動
	camera_.translation_ = {0.0f, 0.0f, -25.0f};

	camera_.UpdateMatrix();

	if (backgroundModel_ == nullptr) 
	{
		backgroundModel_ = Model::CreateFromOBJ("GameOverBackground", true);
	}

	if (wordModel_ == nullptr) 
	{
		wordModel_ = Model::CreateFromOBJ("GameOverWord", true);
	}

	if (spaceWordModel_ == nullptr) 
	{
		spaceWordModel_ = Model::CreateFromOBJ("GameOverSPACEWord", true);
	}

	backgroundTransform_.Initialize();
	wordTransform_.Initialize();
	spaceWordTransform_.Initialize();

	backgroundTransform_.rotation_.y = std::numbers::pi_v<float>;

	wordTransform_.rotation_.y = std::numbers::pi_v<float>;

	spaceWordTransform_.rotation_.y = std::numbers::pi_v<float>;

	backgroundTransform_.scale_ = {1.15f, 1.15f, 1.0f};

	backgroundTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	// 背景よりカメラ側へ
	wordTransform_.translation_ = {0.0f, 0.0f, -0.1f};

	// GAME OVERよりさらにカメラ側へ
	spaceWordTransform_.translation_ = {0.0f, -2.0f, -0.2f};

	backgroundTransform_.matWorld_ = MakeAffineMatrix(backgroundTransform_.scale_, backgroundTransform_.rotation_, backgroundTransform_.translation_);

	wordTransform_.matWorld_ = MakeAffineMatrix(wordTransform_.scale_, wordTransform_.rotation_, wordTransform_.translation_);

	spaceWordTransform_.matWorld_ = MakeAffineMatrix(spaceWordTransform_.scale_, spaceWordTransform_.rotation_, spaceWordTransform_.translation_);

	backgroundTransform_.TransferMatrix();
	wordTransform_.TransferMatrix();
	spaceWordTransform_.TransferMatrix();

	spaceBlinkTimer_ = 0.0f;
	isSpaceWordVisible_ = true;

}

void GameOverScene::Update()
{
	Input* input = Input::GetInstance();

	// 現在のSpaceキー入力
	const bool isSpaceKeyPressed = input->PushKey(DIK_SPACE);

	// Spaceを押した瞬間にタイトルへ戻る
	if (isSpaceKeyPressed && !wasSpaceKeyPressed_) 
	{
		returnTitleRequested_ = true;
	}

	// 次フレーム用
	wasSpaceKeyPressed_ = isSpaceKeyPressed;

	// SPACE案内の点滅
	spaceBlinkTimer_ += 1.0f / 60.0f;

	if (spaceBlinkTimer_ >= kSpaceBlinkInterval) 
	{
		spaceBlinkTimer_ = 0.0f;
		isSpaceWordVisible_ = !isSpaceWordVisible_;
	}

}

void GameOverScene::Draw() 
{
	backgroundModel_->Draw(backgroundTransform_, camera_);

	wordModel_->Draw(wordTransform_, camera_);

	if (isSpaceWordVisible_) 
	{
		spaceWordModel_->Draw(spaceWordTransform_, camera_);
	}
}

GameOverScene::~GameOverScene()
{
	delete backgroundModel_;
	backgroundModel_ = nullptr;

	delete wordModel_;
	wordModel_ = nullptr;

	delete spaceWordModel_;
	spaceWordModel_ = nullptr;
}