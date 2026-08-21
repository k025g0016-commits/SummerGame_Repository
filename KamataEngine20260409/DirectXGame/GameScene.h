#pragma once
#include "KamataEngine.h"
#include <unordered_set>
#include <vector>
#include "CameraController.h"
#include "Enemy.h"
#include "BackGroundWall.h"

// クラスの前方宣言
class Player;
class Boomerang;
class SkyDome;
class MapChipField;
class ArrowEnemy;
class ArrowBullet;
class ShieldEnemy;
class Switch;
class ShutterDoor;
class MoveBlock;
class Spike;
class Boss;

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

	// ゲームオーバーか
	bool IsGameOver() const
	{ 
		return isGameOver_; 
	}

	bool IsGameClear() const 
	{ 
		return isGameClear_; 
	}

private:
	// ブロックの生成
	void GenerateBlocks();

private:
	// カメラ
	KamataEngine::Camera camera_;

	// モデル
	KamataEngine::Model* blockModel_ = nullptr;
	KamataEngine::Model* playerModel_ = nullptr;
	KamataEngine::Model* boomerangModel_ = nullptr;
	KamataEngine::Model* skyDomeModel_ = nullptr;

	// 敵モデル
	KamataEngine::Model* enemyModel_ = nullptr;

	// 敵の剣モデル
	KamataEngine::Model* swordModel_ = nullptr;

	// 遠距離敵モデル
	KamataEngine::Model* arrowEnemyModel_ = nullptr;

	// 遠距離敵の武器モデル
	KamataEngine::Model* arrowModel_ = nullptr;

	// 遠距離敵の弾モデル
	KamataEngine::Model* arrowBulletModel_ = nullptr;

	// 盾敵モデル
	KamataEngine::Model* shieldEnemyModel_ = nullptr;

	// 盾敵が持つ盾モデル
	KamataEngine::Model* enemyShieldModel_ = nullptr;

	// スイッチモデル
	KamataEngine::Model* switchModel_ = nullptr;

	// シャッタードアモデル
	KamataEngine::Model* shutterDoorModel_ = nullptr;

	// 動く床モデル
	KamataEngine::Model* moveBlockModel_ = nullptr;

	// 横レールモデル
	KamataEngine::Model* besideRailModel_ = nullptr;

	// 縦レールモデル
	KamataEngine::Model* verticalRailModel_ = nullptr;

	// ダメージ床モデル
	KamataEngine::Model* spikeBlockModel_ = nullptr;

	// ボスモデル
	KamataEngine::Model* bossModel_ = nullptr;

	// ボスの剣モデル
	KamataEngine::Model* bossSwordModel_ = nullptr;

	// ボスのクロスボウモデル
	KamataEngine::Model* crossbowModel_ = nullptr;

	// 背景壁モデル
	KamataEngine::Model* backGroundWallModel_ = nullptr;

	// ゲームオブジェクト
	Player* player_ = nullptr;
	Boomerang* boomerang_ = nullptr;
	SkyDome* skyDome_ = nullptr;
	MapChipField* mapChipField_ = nullptr;

	// 剣敵
	std::vector<Enemy*> enemies_;

	// 遠距離敵
	std::vector<ArrowEnemy*> arrowEnemies_;

	// 遠距離敵が発射した弾
	std::vector<ArrowBullet*> arrowBullets_;

	// 盾敵
	std::vector<ShieldEnemy*> shieldEnemies_;

	// ブロックのワールド変換
	std::vector<std::vector<KamataEngine::WorldTransform*>> worldTransformBlocks_;

	// 前フレームでZキーが押されていたか
	bool wasThrowKeyPressed_ = false;

	// 行きの盾が敵へ命中済みか
	std::unordered_set<Enemy*> hitEnemiesOutbound_;

	// 帰りの盾が敵へ命中済みか
	std::unordered_set<Enemy*> hitEnemiesReturn_;

	// 行きの盾が遠距離敵へ命中済みか
	std::unordered_set<ArrowEnemy*> hitArrowEnemiesOutbound_;

	// 帰りの盾が遠距離敵へ命中済みか
	std::unordered_set<ArrowEnemy*> hitArrowEnemiesReturn_;

	// カメラコントローラー
	CameraController* cameraController_ = nullptr;

	std::unordered_set<ShieldEnemy*> hitShieldEnemiesOutbound_;
	std::unordered_set<ShieldEnemy*> hitShieldEnemiesReturn_;

	// スイッチ
	std::vector<Switch*> switches_;

	// シャッタードア
	std::vector<ShutterDoor*> shutterDoors_;

	// 動く床
	std::vector<MoveBlock*> moveBlocks_;

	// 横レールのワールド変換
	std::vector<KamataEngine::WorldTransform*> worldTransformBesideRails_;

	// 縦レールのワールド変換
	std::vector<KamataEngine::WorldTransform*> worldTransformVerticalRails_;

	// ダメージ床
	std::vector<Spike*> spikes_;

	// ボス
	Boss* boss_ = nullptr;

	// 行きのブーメランがボスへ命中済みか
	bool hitBossOutbound_ = false;

	// 帰りのブーメランがボスへ命中済みか
	bool hitBossReturn_ = false;

	// ボスエリアの境界
	std::vector<KamataEngine::Vector3> bossAreaPositions_;

	// ボスエリアの左端・右端
	float bossAreaLeft_ = 0.0f;
	float bossAreaRight_ = 0.0f;

	// ボスエリアに入ったか
	bool isBossAreaEntered_ = false;

	// 通常時のカメラ移動範囲
	CameraController::Rect normalCameraArea_{};

	// ゲームオーバーフラグ
	bool isGameOver_ = false;

	BackGroundWall* backGroundWall_ = nullptr;

	bool isGameClear_ = false;

};