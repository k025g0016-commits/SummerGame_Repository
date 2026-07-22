#pragma once
#include "KamataEngine.h"
#include <vector>

// クラスの前方宣言
class Player;
class Boomerang;
class SkyDome;
class MapChipField;

// ゲームシーン
class GameScene 
{
public:
	// 初期化
	void Initialize();

	// 更新
	void Update();

	// 描画
	void Draw();

	// デストラクタ
	~GameScene();

private:
	// ブロックの生成
	void GenerateBlocks();

	// カメラ更新
	void UpdateCamera();

private:
	// カメラ
	KamataEngine::Camera camera_;

	// モデル
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* boomerangModel_ = nullptr;
	KamataEngine::Model* skyDomeModel_ = nullptr;

	// ゲームオブジェクト
	Player* player_ = nullptr;
	Boomerang* boomerang_ = nullptr;
	SkyDome* skyDome_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	// ブロックのワールド変換
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// 前フレームでJキーが押されていたか
	bool wasThrowKeyPressed_ = false;

};