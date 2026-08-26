#include "TitleScene.h"
#include "MathUtility.h"

using namespace KamataEngine;

void TitleScene::Initialize() 
{
	startRequested_ = false;
	wasSpaceKeyPressed_ = false;

	spaceBlinkTimer_ = 0.0f;
	isSpaceWordVisible_ = true;

	// カメラ初期化
	camera_.Initialize();

	// GameOver / GameClearと同じ距離を基準にする
	camera_.translation_ = {0.0f, 0.0f, -25.0f};
	camera_.UpdateMatrix();

	// モデル読み込み
	// Initializeが何度呼ばれても再読み込みしない
	if (backgroundModel_ == nullptr)
	{
		backgroundModel_ = Model::CreateFromOBJ("TitleBackground", true);
	}

	if (titleWordModel_ == nullptr)
	{
		titleWordModel_ = Model::CreateFromOBJ("TitleWord", true);
	}

	if (pressSpaceModel_ == nullptr) 
	{
		pressSpaceModel_ = Model::CreateFromOBJ("TitlePressSPACE", true);
	}

	if (playerModel_ == nullptr)
	{
		playerModel_ = Model::CreateFromOBJ("player", true);
	}

	if (shieldModel_ == nullptr)
	{
		shieldModel_ = Model::CreateFromOBJ("Shild", true);
	}

	// Transform初期化
	backgroundTransform_.Initialize();
	titleWordTransform_.Initialize();
	pressSpaceTransform_.Initialize();
	playerTransform_.Initialize();
	shieldTransform_.Initialize();
	isTitleBGMStarted_ = false;
	isTitleBGMFadingOut_ = false;
	titleBGMFadeTimer_ = 0.0f;

	// 面をカメラ側へ向ける
	backgroundTransform_.rotation_.y = std::numbers::pi_v<float>;

	titleWordTransform_.rotation_.y = std::numbers::pi_v<float>;

	pressSpaceTransform_.rotation_.y = std::numbers::pi_v<float>;

	// 背景サイズ
	backgroundTransform_.scale_ = {1.15f, 1.15f, 1.0f};

	// 背景
	backgroundTransform_.translation_ = {0.0f, 0.0f, 0.0f};

	// タイトル文字を背景より手前へ
	titleWordTransform_.translation_ = {0.0f, 0.0f, -0.1f};

	// SPACE案内をさらに手前へ
	pressSpaceTransform_.translation_ = {0.0f, 0.0f, -0.2f};

	// プレイヤー
	playerFloatTimer_ = 0.0f;
	playerBaseY_ = -0.5f;
	playerTransform_.translation_ = {0.0f, playerBaseY_, -15.0f};
	playerTransform_.scale_ = {2.0f, 2.0f, 2.0f};
	playerTransform_.rotation_.x = std::numbers::pi_v<float>;
	playerTransform_.rotation_.y = 0.0f;
	playerTransform_.rotation_.z = std::numbers::pi_v<float>;

	// 盾
	shieldRotationAngle_ = 0.0f;
	shieldTransform_.translation_ = {0.0f, -0.5f, -15.0f};
	shieldTransform_.scale_ = {2.0f, 2.0f, 2.0f};
	shieldTransform_.rotation_.x = std::numbers::pi_v<float>;
	shieldTransform_.rotation_.y = shieldRotationAngle_;
	shieldTransform_.rotation_.z = std::numbers::pi_v<float>;
	
	// 行列作成
	backgroundTransform_.matWorld_ = MakeAffineMatrix(backgroundTransform_.scale_, backgroundTransform_.rotation_, backgroundTransform_.translation_);

	titleWordTransform_.matWorld_ = MakeAffineMatrix(titleWordTransform_.scale_, titleWordTransform_.rotation_, titleWordTransform_.translation_);

	pressSpaceTransform_.matWorld_ = MakeAffineMatrix(pressSpaceTransform_.scale_, pressSpaceTransform_.rotation_, pressSpaceTransform_.translation_);

	backgroundTransform_.TransferMatrix();
	titleWordTransform_.TransferMatrix();
	pressSpaceTransform_.TransferMatrix();

	playerTransform_.matWorld_ = MakeAffineMatrix(playerTransform_.scale_, playerTransform_.rotation_, playerTransform_.translation_);

	shieldTransform_.matWorld_ = MakeAffineMatrix(shieldTransform_.scale_, shieldTransform_.rotation_, shieldTransform_.translation_);

	playerTransform_.TransferMatrix();
	shieldTransform_.TransferMatrix();

	Audio* audio = Audio::GetInstance();

	titleBGMHandle_ = audio->LoadWave("BGM/Title.wav");

	// Initializeではまだ再生しない
	isTitleBGMStarted_ = false;

}

void TitleScene::Update() 
{
	// タイトルBGMを1回だけ開始
	if (!isTitleBGMStarted_) 
	{
		isTitleBGMStarted_ = true;

		titleBGMVoiceHandle_ = Audio::GetInstance()->PlayWave(titleBGMHandle_, true, kTitleBGMVolume);
	}

	Input* input = Input::GetInstance();

	// 現在のSpaceキー入力
	const bool isSpaceKeyPressed = input->PushKey(DIK_SPACE);

	// Spaceを押した瞬間にゲーム開始
	if (isSpaceKeyPressed && !wasSpaceKeyPressed_)
	{
		// タイトルBGMのフェードアウト開始
		if (isTitleBGMStarted_) 
		{
			isTitleBGMFadingOut_ = true;
			titleBGMFadeTimer_ = 0.0f;
		}

		startRequested_ = true;
	}

	wasSpaceKeyPressed_ = isSpaceKeyPressed;

	// BGMフェード更新
	UpdateBGMFade();

	// SPACE案内の点滅
	spaceBlinkTimer_ += 1.0f / 60.0f;

	if (spaceBlinkTimer_ >= kSpaceBlinkInterval) 
	{
		spaceBlinkTimer_ = 0.0f;

		isSpaceWordVisible_ = !isSpaceWordVisible_;
	}

	// 盾をプレイヤーの周囲に回転させる
	shieldRotationAngle_ += kShieldRotationSpeed;

	// 1周したら角度を戻す
	if (shieldRotationAngle_ >= 2.0f * std::numbers::pi_v<float>) 
	{
		shieldRotationAngle_ -= 2.0f * std::numbers::pi_v<float>;
	}

	// 回転角を盾に反映
	shieldTransform_.rotation_.x = std::numbers::pi_v<float>;
	shieldTransform_.rotation_.y = shieldRotationAngle_;
	shieldTransform_.rotation_.z = std::numbers::pi_v<float>;

	// ワールド行列を更新
	shieldTransform_.matWorld_ = MakeAffineMatrix(shieldTransform_.scale_, shieldTransform_.rotation_, shieldTransform_.translation_);

	shieldTransform_.TransferMatrix();

	playerFloatTimer_ += 1.0f / 60.0f;

	playerTransform_.translation_.y = playerBaseY_ + std::sin(playerFloatTimer_ * kPlayerFloatSpeed) * kPlayerFloatRange;

	// プレイヤーの行列を更新
	playerTransform_.matWorld_ = MakeAffineMatrix(playerTransform_.scale_, playerTransform_.rotation_, playerTransform_.translation_);

	playerTransform_.TransferMatrix();

}

void TitleScene::Draw() 
{
	backgroundModel_->Draw(backgroundTransform_, camera_);

	titleWordModel_->Draw(titleWordTransform_, camera_);

	// プレイヤー
	if (playerModel_ != nullptr) 
	{
		playerModel_->Draw(playerTransform_, camera_);
	}

	// 盾
	if (shieldModel_ != nullptr)
	{
		shieldModel_->Draw(shieldTransform_, camera_);
	}

	if (isSpaceWordVisible_)
	{
		pressSpaceModel_->Draw(pressSpaceTransform_, camera_);
	}
}

void TitleScene::StopBGM()
{
	if (isTitleBGMStarted_) 
	{
		Audio::GetInstance()->StopWave(titleBGMVoiceHandle_);

		isTitleBGMStarted_ = false;
	}
}

void TitleScene::UpdateBGMFade()
{
	if (!isTitleBGMFadingOut_)
	{
		return;
	}

	constexpr float kDeltaTime = 1.0f / 60.0f;

	titleBGMFadeTimer_ += kDeltaTime;

	float progress = titleBGMFadeTimer_ / kTitleBGMFadeDuration;

	if (progress > 1.0f)
	{
		progress = 1.0f;
	}

	// 通常音量から0へ徐々に下げる
	const float volume = kTitleBGMVolume * (1.0f - progress);

	Audio::GetInstance()->SetVolume(titleBGMVoiceHandle_, volume);

	// フェード終了
	if (titleBGMFadeTimer_ >= kTitleBGMFadeDuration)
	{
		Audio::GetInstance()->StopWave(titleBGMVoiceHandle_);

		titleBGMVoiceHandle_ = 0;
		isTitleBGMStarted_ = false;
		isTitleBGMFadingOut_ = false;
	}
}

TitleScene::~TitleScene()
{
	StopBGM();

	delete backgroundModel_;
	backgroundModel_ = nullptr;

	delete titleWordModel_;
	titleWordModel_ = nullptr;

	delete pressSpaceModel_;
	pressSpaceModel_ = nullptr;

	delete playerModel_;
	playerModel_ = nullptr;

	delete shieldModel_;
	shieldModel_ = nullptr;

}