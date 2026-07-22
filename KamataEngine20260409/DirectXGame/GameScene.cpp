#include "GameScene.h"
#include "Boomerang.h"
#include "MapChipField.h"
#include "MathUtility.h"
#include "Player.h"
#include "Skydome.h"

using namespace KamataEngine;

void GameScene::Initialize() 
{
	// カメラ初期化
	camera_.Initialize();

	// 描画範囲
	camera_.farZ = 5000.0f;

	// モデル読み込み
	blockModel_ = Model::CreateFromOBJ("block", true);
	playerModel_ = Model::CreateFromOBJ("player", true);
	boomerangModel_ = Model::CreateFromOBJ("ring", true);
	skyDomeModel_ = Model::CreateFromOBJ("SkyDome", true);

	// マップチップ生成
	mapChipField_ = new MapChipField();

	mapChipField_->LoadMapChipCsv("Resources/MapChip/TestMap.csv");

	// ブロック生成
	GenerateBlocks();

	// プレイヤー生成
	player_ = new Player();

	player_->Initialize(playerModel_, &camera_, mapChipField_->GetPlayerPosition());

	player_->SetMapChipField(mapChipField_);

	// ブーメラン生成
	boomerang_ = new Boomerang();

	boomerang_->Initialize(boomerangModel_, &camera_, player_);

	// 天球生成
	skyDome_ = new SkyDome();

	skyDome_->Initialize(skyDomeModel_, &camera_);

	// 最初のカメラ位置を設定
	UpdateCamera();
}

void GameScene::Update()
{
	Input* input = Input::GetInstance();

	// プレイヤー更新
	player_->Update();

	// 現在のJキー入力
	const bool isThrowKeyPressed = input->PushKey(DIK_J);

	// 今押されていて、前フレームでは押されていなかった
	if (isThrowKeyPressed && !wasThrowKeyPressed_)
	{
		boomerang_->Throw();
	}

	// 現在の入力を次フレーム用に保存
	wasThrowKeyPressed_ = isThrowKeyPressed;

	// ブーメラン更新
	boomerang_->Update();

	// 天球更新
	skyDome_->Update();

	// カメラ更新
	UpdateCamera();
}

void GameScene::Draw()
{
	// 天球
	skyDome_->Draw();

	// プレイヤー
	player_->Draw();

	// ブーメラン
	boomerang_->Draw();

	// ブロック
	for (const auto& blockLine : worldTransformBlocks_) 
	{
		for (WorldTransform* worldTransform : blockLine)
		{
			if (worldTransform == nullptr) 
			{
				continue;
			}

			blockModel_->Draw(*worldTransform, camera_);
		}
	}
}

GameScene::~GameScene()
{
	delete player_;
	player_ = nullptr;

	delete boomerang_;
	boomerang_ = nullptr;

	delete skyDome_;
	skyDome_ = nullptr;

	delete mapChipField_;
	mapChipField_ = nullptr;

	for (auto& blockLine : worldTransformBlocks_)
	{
		for (WorldTransform*& worldTransform : blockLine) 
		{
			delete worldTransform;
			worldTransform = nullptr;
		}
	}

	worldTransformBlocks_.clear();

	delete blockModel_;
	blockModel_ = nullptr;

	delete playerModel_;
	playerModel_ = nullptr;

	delete boomerangModel_;
	boomerangModel_ = nullptr;

	delete skyDomeModel_;
	skyDomeModel_ = nullptr;
}

void GameScene::GenerateBlocks()
{
	const uint32_t numVertical = mapChipField_->GetNumBlockVertical();

	const uint32_t numHorizontal = mapChipField_->GetNumBlockHorizontal();

	worldTransformBlocks_.resize(numVertical);

	for (uint32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		worldTransformBlocks_[yIndex].resize(numHorizontal, nullptr);

		for (uint32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (mapChipField_->GetMapChipTypeByIndex(static_cast<int32_t>(xIndex), static_cast<int32_t>(yIndex)) != MapChipType::kBlock) 
			{
				continue;
			}

			WorldTransform* worldTransform = new WorldTransform();

			worldTransform->Initialize();

			worldTransform->translation_ = mapChipField_->GetMapChipPositionByIndex(xIndex, yIndex);

			worldTransform->matWorld_ = MakeAffineMatrix(worldTransform->scale_, worldTransform->rotation_, worldTransform->translation_);

			worldTransform->TransferMatrix();

			worldTransformBlocks_[yIndex][xIndex] = worldTransform;
		}
	}
}

void GameScene::UpdateCamera()
{
	/* const Vector3 playerPosition = player_->GetWorldPosition();

	// 2D横スクロール風のカメラ位置
	camera_.translation_ = {playerPosition.x, playerPosition.y + 3.0f, playerPosition.z - 15.0f};

	camera_.rotation_ = {0.0f, 0.0f, 0.0f};*/

	// 動作確認のためカメラを固定
	camera_.translation_ = {5.0f, 5.0f, -15.0f};

	camera_.rotation_ = {0.0f, 0.0f, 0.0f};

	camera_.UpdateMatrix();
}