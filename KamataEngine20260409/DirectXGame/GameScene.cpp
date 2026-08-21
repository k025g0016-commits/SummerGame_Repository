#include "GameScene.h"
#include "Boomerang.h"
#include "MapChipField.h"
#include "MathUtility.h"
#include "Player.h"
#include "Skydome.h"
#include <cmath>
#include "ArrowEnemy.h"
#include "ArrowBullet.h"
#include <algorithm>
#include "ShieldEnemy.h"
#include "Switch.h"
#include "ShutterDoor.h"
#include "MoveBlock.h" 
#include "Spike.h"
#include "Boss.h"
#include "BackGroundWall.h"

using namespace KamataEngine;

namespace 
{
// ガード成功時に敵を押し返す距離
constexpr float kGuardPushBackDistance = 0.3f;
constexpr float kChargePlayerPushBackDistance = 0.35f;

// 敵とブーメランの接触判定
bool IsEnemyBoomerangCollision(const Vector3& enemyPosition, const Vector3& boomerangPosition)
{
	// 敵の当たり判定の半分の大きさ
	constexpr float kEnemyHalfWidth = 0.4f;
	constexpr float kEnemyHalfHeight = 0.4f;

	// ブーメランの当たり判定の半分の大きさ
	constexpr float kBoomerangHalfWidth = 0.3f;
	constexpr float kBoomerangHalfHeight = 0.3f;

	// X方向の距離
	const float distanceX = std::abs(enemyPosition.x - boomerangPosition.x);

	// Y方向の距離
	const float distanceY = std::abs(enemyPosition.y - boomerangPosition.y);

	// X・Yの両方が範囲内なら接触
	return distanceX <= kEnemyHalfWidth + kBoomerangHalfWidth && distanceY <= kEnemyHalfHeight + kBoomerangHalfHeight;
}

// プレイヤーと敵の接触判定
bool IsPlayerEnemyCollision(const Vector3& playerPosition, const Vector3& enemyPosition) 
{
	// プレイヤーの当たり判定の半分の大きさ
	constexpr float kPlayerHalfWidth = 0.4f;
	constexpr float kPlayerHalfHeight = 0.4f;

	// 敵の当たり判定の半分の大きさ
	constexpr float kEnemyHalfWidth = 0.4f;
	constexpr float kEnemyHalfHeight = 0.4f;

	// X方向の距離
	const float distanceX = std::abs(playerPosition.x - enemyPosition.x);

	// Y方向の距離
	const float distanceY = std::abs(playerPosition.y - enemyPosition.y);

	// X方向とY方向の両方で重なっていれば接触
	return distanceX <= kPlayerHalfWidth + kEnemyHalfWidth && distanceY <= kPlayerHalfHeight + kEnemyHalfHeight;
}

// 敵がプレイヤーの正面にいるか
bool IsEnemyInFront(const Vector3& playerPosition, const Vector3& enemyPosition, Player::LRDirection playerDirection)
{
	if (playerDirection == Player::LRDirection::kRight)
	{
		// 右向きなら、敵がプレイヤーの右側にいる
		return enemyPosition.x >= playerPosition.x;
	}
	else 
	{
		// 左向きなら、敵がプレイヤーの左側にいる
		return enemyPosition.x <= playerPosition.x;
	}
}

// プレイヤーとArrowBulletの接触判定
bool IsPlayerArrowBulletCollision(const Vector3& playerPosition, const Vector3& bulletPosition) 
{
	// プレイヤーの当たり判定の半分
	constexpr float kPlayerHalfWidth = 0.4f;
	constexpr float kPlayerHalfHeight = 0.4f;

	// 弾の当たり判定の半分
	constexpr float kBulletHalfWidth = 0.2f;
	constexpr float kBulletHalfHeight = 0.2f;

	const float distanceX = std::abs(playerPosition.x - bulletPosition.x);

	const float distanceY = std::abs(playerPosition.y - bulletPosition.y);

	return distanceX <= kPlayerHalfWidth + kBulletHalfWidth && distanceY <= kPlayerHalfHeight + kBulletHalfHeight;
}

// スイッチとブーメランの接触判定
bool IsSwitchBoomerangCollision(const Vector3& switchPosition, const Vector3& boomerangPosition)
{
	// スイッチの当たり判定の半分
	constexpr float kSwitchHalfWidth = 0.4f;
	constexpr float kSwitchHalfHeight = 0.4f;

	// ブーメランの当たり判定の半分
	constexpr float kBoomerangHalfWidth = 0.3f;
	constexpr float kBoomerangHalfHeight = 0.3f;

	const float distanceX = std::abs(switchPosition.x - boomerangPosition.x);

	const float distanceY = std::abs(switchPosition.y - boomerangPosition.y);

	return distanceX <= kSwitchHalfWidth + kBoomerangHalfWidth && distanceY <= kSwitchHalfHeight + kBoomerangHalfHeight;
}

// プレイヤーとシャッタードアの接触判定
bool IsPlayerShutterDoorCollision(const Vector3& playerPosition, const Vector3& doorPosition)
{
	// プレイヤーの当たり判定の半分
	constexpr float kPlayerHalfWidth = 0.4f;
	constexpr float kPlayerHalfHeight = 0.4f;

	// シャッタードアの当たり判定の半分
	constexpr float kDoorHalfWidth = 0.5f;
	constexpr float kDoorHalfHeight = 0.5f;

	const float distanceX = std::abs(playerPosition.x - doorPosition.x);

	const float distanceY = std::abs(playerPosition.y - doorPosition.y);

	return distanceX <= kPlayerHalfWidth + kDoorHalfWidth && distanceY <= kPlayerHalfHeight + kDoorHalfHeight;
}

// プレイヤーがMoveBlockの上に乗っているか
bool IsPlayerOnMoveBlock(const Vector3& playerPosition, const Vector3& moveBlockPosition)
{
	// プレイヤーの半分の大きさ
	constexpr float kPlayerHalfWidth = 0.4f;
	constexpr float kPlayerHalfHeight = 0.4f;

	// MoveBlockの半分の大きさ
	constexpr float kMoveBlockHalfWidth = 0.5f;
	constexpr float kMoveBlockHalfHeight = 0.5f;

	// X方向で重なっているか
	const float distanceX = std::abs(playerPosition.x - moveBlockPosition.x);

	if (distanceX > kPlayerHalfWidth + kMoveBlockHalfWidth)
	{
		return false;
	}

	// MoveBlockの上面
	const float moveBlockTop = moveBlockPosition.y + kMoveBlockHalfHeight;

	// プレイヤーの足元
	const float playerBottom = playerPosition.y - kPlayerHalfHeight;

	// 足元とMoveBlock上面の差
	const float distanceY = std::abs(playerBottom - moveBlockTop);

	// 少し余裕を持たせる
	constexpr float kLandingTolerance = 0.1f;

	return distanceY <= kLandingTolerance;
}

// プレイヤーとダメージ床の接触判定
bool IsPlayerSpikeCollision(const Vector3& playerPosition, const Vector3& spikePosition) 
{
	// プレイヤーの当たり判定の半分
	constexpr float kPlayerHalfWidth = 0.4f;
	constexpr float kPlayerHalfHeight = 0.4f;

	// SpikeBlockの当たり判定の半分
	constexpr float kSpikeHalfWidth = 0.5f;
	constexpr float kSpikeHalfHeight = 0.5f;

	const float distanceX = std::abs(playerPosition.x - spikePosition.x);

	const float distanceY = std::abs(playerPosition.y - spikePosition.y);

	return distanceX <= kPlayerHalfWidth + kSpikeHalfWidth && distanceY <= kPlayerHalfHeight + kSpikeHalfHeight;
}

// ボスとブーメランの接触判定
bool IsBossBoomerangCollision(const Vector3& bossPosition, const Vector3& boomerangPosition)
{
	// ボスの当たり判定
	constexpr float kBossHalfWidth = 1.0f;
	constexpr float kBossHalfHeight = 1.0f;

	// ブーメランの当たり判定
	constexpr float kBoomerangHalfWidth = 0.3f;
	constexpr float kBoomerangHalfHeight = 0.3f;

	const float distanceX = std::abs(bossPosition.x - boomerangPosition.x);

	const float distanceY = std::abs(bossPosition.y - boomerangPosition.y);

	return distanceX <= kBossHalfWidth + kBoomerangHalfWidth && distanceY <= kBossHalfHeight + kBoomerangHalfHeight;
}

// プレイヤーとボス本体の接触判定
bool IsPlayerBossCollision(const Vector3& playerPosition, const Vector3& bossPosition)
{
	// プレイヤーの当たり判定の半分
	constexpr float kPlayerHalfWidth = 0.4f;
	constexpr float kPlayerHalfHeight = 0.4f;

	// ボスの当たり判定の半分
	constexpr float kBossHalfWidth = 0.9f;
	constexpr float kBossHalfHeight = 0.8f;

	const float distanceX = std::abs(playerPosition.x - bossPosition.x);
	const float distanceY = std::abs(playerPosition.y - bossPosition.y);

	return distanceX <= kPlayerHalfWidth + kBossHalfWidth && distanceY <= kPlayerHalfHeight + kBossHalfHeight;
}

// ブーメランとMoveBlockの接触判定
bool IsBoomerangMoveBlockCollision(const Vector3& boomerangPosition, const Vector3& moveBlockPosition) 
{
	// ブーメランの当たり判定
	constexpr float kBoomerangHalfWidth = 0.3f;
	constexpr float kBoomerangHalfHeight = 0.3f;

	// MoveBlockの当たり判定
	constexpr float kMoveBlockHalfWidth = 0.5f;
	constexpr float kMoveBlockHalfHeight = 0.5f;

	const float distanceX = std::abs(boomerangPosition.x - moveBlockPosition.x);

	const float distanceY = std::abs(boomerangPosition.y - moveBlockPosition.y);

	return distanceX <= kBoomerangHalfWidth + kMoveBlockHalfWidth && distanceY <= kBoomerangHalfHeight + kMoveBlockHalfHeight;
}

} // namespace

void GameScene::Initialize() 
{
	isGameOver_ = false;
	isGameClear_ = false;

	// カメラ初期化
	camera_.Initialize();

	// 描画範囲
	camera_.farZ = 5000.0f;

	// モデル読み込み
	blockModel_ = Model::CreateFromOBJ("block", true);
	playerModel_ = Model::CreateFromOBJ("player", true);
	enemyModel_ = Model::CreateFromOBJ("enemy", true);
	swordModel_ = Model::CreateFromOBJ("Sword", true);
	arrowEnemyModel_ = Model::CreateFromOBJ("ArrowEnemy", true);
	arrowModel_ = Model::CreateFromOBJ("Arrow", true);
	arrowBulletModel_ = Model::CreateFromOBJ("ArrowBullet", true);
	boomerangModel_ = Model::CreateFromOBJ("ring", true);
	skyDomeModel_ = Model::CreateFromOBJ("SkyDome", true);
	shieldEnemyModel_ = Model::CreateFromOBJ("ShieldEnemy", true);
	enemyShieldModel_ = Model::CreateFromOBJ("E_Shield", true);
	switchModel_ = Model::CreateFromOBJ("Switch", true);
	shutterDoorModel_ = Model::CreateFromOBJ("ShutterDoor", true);
	moveBlockModel_ = Model::CreateFromOBJ("MoveBlock", true);
	besideRailModel_ = Model::CreateFromOBJ("BesideRail", true);
	verticalRailModel_ = Model::CreateFromOBJ("VerticalRail", true);
	spikeBlockModel_ = Model::CreateFromOBJ("SpikeBlock", true);
	bossModel_ = Model::CreateFromOBJ("Boss", true);
	bossSwordModel_ = Model::CreateFromOBJ("BossSword", true);
	crossbowModel_ = Model::CreateFromOBJ("Crossbow", true);
	backGroundWallModel_ = Model::CreateFromOBJ("BackgroundWall", true);

	// マップチップ生成
	mapChipField_ = new MapChipField();

	mapChipField_->LoadMapChipCsv("Resources/MapChip/TestMap.csv");

	// ブロック生成
	GenerateBlocks();

	// プレイヤー生成
	player_ = new Player();

	player_->Initialize(playerModel_, &camera_, mapChipField_->GetPlayerPosition());

	player_->SetMapChipField(mapChipField_);

	const std::vector<Vector3> enemyPositions = mapChipField_->GetEnemyPositions();

	for (const Vector3& position : enemyPositions) 
	{
		Enemy* enemy = new Enemy();

		enemy->Initialize(enemyModel_, swordModel_, &camera_, position, player_);

		enemy->SetMapChipField(mapChipField_);

		enemies_.push_back(enemy);
	}

	// 遠距離敵の生成
	const std::vector<Vector3> arrowEnemyPositions = mapChipField_->GetArrowEnemyPositions();

	for (const Vector3& position : arrowEnemyPositions) 
	{
		ArrowEnemy* arrowEnemy = new ArrowEnemy();

		arrowEnemy->Initialize(arrowEnemyModel_, arrowModel_, &camera_, position, player_);

		arrowEnemy->SetMapChipField(mapChipField_);

		arrowEnemies_.push_back(arrowEnemy);
	}

	// 盾敵の生成
	const std::vector<Vector3> shieldEnemyPositions = mapChipField_->GetShieldEnemyPositions();

	for (const Vector3& position : shieldEnemyPositions)
	{
		ShieldEnemy* shieldEnemy = new ShieldEnemy();

		shieldEnemy->Initialize(shieldEnemyModel_, enemyShieldModel_, &camera_, position, player_);

		shieldEnemy->SetMapChipField(mapChipField_);

		shieldEnemies_.push_back(shieldEnemy);
	}

	const std::vector<Vector3> switchPositions = mapChipField_->GetSwitchPositions();

	for (const Vector3& position : switchPositions) 
	{
		Switch* switchObject = new Switch();

		switchObject->Initialize(switchModel_, &camera_, position);

		switches_.push_back(switchObject);
	}

	// シャッタードアの生成
	const std::vector<Vector3> shutterDoorPositions = mapChipField_->GetShutterDoorPositions();

	for (const Vector3& position : shutterDoorPositions) 
	{
		ShutterDoor* shutterDoor = new ShutterDoor();

		shutterDoor->Initialize(shutterDoorModel_, &camera_, position);

		shutterDoors_.push_back(shutterDoor);
	}

	// 動く床の生成
	const std::vector<Vector3> moveBlockPositions = mapChipField_->GetMoveBlockPositions();

	for (const Vector3& position : moveBlockPositions) 
	{
		MoveBlock* moveBlock = new MoveBlock();

		moveBlock->Initialize(moveBlockModel_, &camera_, position);

		moveBlock->SetMapChipField(mapChipField_);

		moveBlocks_.push_back(moveBlock);
	}

	// 横レールの生成
	const std::vector<Vector3> besideRailPositions = mapChipField_->GetBesideRailPositions();

	for (const Vector3& position : besideRailPositions)
	{
		WorldTransform* worldTransform = new WorldTransform();

		worldTransform->Initialize();

		worldTransform->translation_ = position;

		worldTransform->matWorld_ = MakeAffineMatrix(worldTransform->scale_, worldTransform->rotation_, worldTransform->translation_);

		worldTransform->TransferMatrix();

		worldTransformBesideRails_.push_back(worldTransform);
	}

	// 縦レールの生成
	const std::vector<Vector3> verticalRailPositions = mapChipField_->GetVerticalRailPositions();

	for (const Vector3& position : verticalRailPositions) 
	{
		WorldTransform* worldTransform = new WorldTransform();

		worldTransform->Initialize();

		worldTransform->translation_ = position;

		worldTransform->matWorld_ = MakeAffineMatrix(worldTransform->scale_, worldTransform->rotation_, worldTransform->translation_);

		worldTransform->TransferMatrix();

		worldTransformVerticalRails_.push_back(worldTransform);
	}

	// ダメージ床の生成
	const std::vector<Vector3> spikePositions = mapChipField_->GetSpikeBlockPositions();

	for (const Vector3& position : spikePositions)
	{
		Spike* spike = new Spike();

		spike->Initialize(spikeBlockModel_, &camera_, position);

		spikes_.push_back(spike);
	}

	// ボス生成
	boss_ = new Boss();

	boss_->Initialize(bossModel_, bossSwordModel_, crossbowModel_, &camera_, mapChipField_->GetBossPosition(), player_);

	boss_->SetMapChipField(mapChipField_);

	bossAreaPositions_ = mapChipField_->GetBossAreaPositions();

	// A0が2つ以上ある場合、左右の境界を求める
	if (bossAreaPositions_.size() >= 2)
	{
		bossAreaLeft_ = bossAreaPositions_[0].x;
		bossAreaRight_ = bossAreaPositions_[0].x;

		for (const Vector3& position : bossAreaPositions_) 
		{
			if (position.x < bossAreaLeft_)
			{
				bossAreaLeft_ = position.x;
			}

			if (position.x > bossAreaRight_) 
			{
				bossAreaRight_ = position.x;
			}
		}
	}

	hitEnemiesOutbound_.clear();
	hitEnemiesReturn_.clear();

	// カメラコントローラーの生成
	cameraController_ = new CameraController();

	// 通常時のカメラ移動可能範囲
	normalCameraArea_.left = 0.0f;
	normalCameraArea_.right = static_cast<float>(mapChipField_->GetNumBlockHorizontal() - 1);
	normalCameraArea_.bottom = 0.0f;
	normalCameraArea_.top = 10.0f;

	cameraController_->SetMovableArea(normalCameraArea_);

	// カメラコントローラーの初期化
	cameraController_->Initialize(&camera_, player_);

	// ブーメラン生成
	boomerang_ = new Boomerang();

	boomerang_->Initialize(boomerangModel_, &camera_, player_);

	boomerang_->SetMapChipField(mapChipField_);

	// 天球生成
	skyDome_ = new SkyDome();

	skyDome_->Initialize(skyDomeModel_, &camera_);

	backGroundWall_ = new BackGroundWall();

	backGroundWall_->Initialize(backGroundWallModel_, &camera_);

}

void GameScene::Update()
{
	Input* input = Input::GetInstance();

	// プレイヤー更新
	player_->Update();

	// プレイヤーが死亡したらゲームオーバー
	if (player_->IsDead())
	{
		isGameOver_ = true;
		return;
	}

	// ボスエリア侵入判定
	if (!isBossAreaEntered_ && player_ != nullptr && !player_->IsDead()) 
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		if (playerPosition.x >= bossAreaLeft_ && playerPosition.x <= bossAreaRight_) 
		{
			isBossAreaEntered_ = true;

			// カメラをボスエリア内に制限
			cameraController_->SetBossArea(bossAreaLeft_, bossAreaRight_);
		}
	}

	// カメラ追従更新
	cameraController_->Update();

	//剣敵の更新
	for (Enemy* enemy : enemies_) 
	{
		if (enemy == nullptr)
		{
			continue;
		}

		enemy->Update();
	}

	// 遠距離敵の更新
	for (ArrowEnemy* arrowEnemy : arrowEnemies_)
	{
		if (arrowEnemy == nullptr)
		{
			continue;
		}

		arrowEnemy->Update();
	}

	// 盾敵の更新
	for (ShieldEnemy* shieldEnemy : shieldEnemies_) 
	{
		if (shieldEnemy == nullptr)
		{
			continue;
		}

		shieldEnemy->Update();
	}

	// スイッチ更新
	for (Switch* switchObject : switches_) 
	{
		if (switchObject == nullptr)
		{
			continue;
		}

		switchObject->Update();
	}

	// スイッチがONになっているか確認
	bool isAnySwitchOn = false;

	for (Switch* switchObject : switches_)
	{
		if (switchObject == nullptr) 
		{
			continue;
		}

		if (switchObject->IsOn()) 
		{
			isAnySwitchOn = true;
			break;
		}
	}

	// スイッチがONならシャッタードアを開く
	if (isAnySwitchOn)
	{
		for (ShutterDoor* shutterDoor : shutterDoors_) 
		{
			if (shutterDoor == nullptr) 
			{
				continue;
			}

			shutterDoor->Open();
		}
	}

	// シャッタードア更新
	for (ShutterDoor* shutterDoor : shutterDoors_) 
	{
		if (shutterDoor == nullptr)
		{
			continue;
		}

		shutterDoor->Update();
	}

	// ダメージ床更新
	for (Spike* spike : spikes_)
	{
		if (spike == nullptr)
		{
			continue;
		}

		spike->Update();
	}

	// ボス更新
	if (boss_ != nullptr)
	{
		boss_->Update();
	}

	// ボスを倒したらゲームクリア
	if (boss_ != nullptr && boss_->IsDead())
	{
		isGameClear_ = true;
		return;
	}

	// プレイヤーと剣敵本体の接触判定
	if (player_ != nullptr && !player_->IsDead()) 
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		for (Enemy* enemy : enemies_)
		{
			if (enemy == nullptr || enemy->IsDead())
			{
				continue;
			}

			const Vector3 enemyPosition = enemy->GetWorldPosition();

			if (!IsPlayerEnemyCollision(playerPosition, enemyPosition))
			{
				continue;
			}

			// 敵本体に接触したので1ダメージ
			player_->OnHit(1);

			if (player_->IsDead()) 
			{
				break;
			}
		}
	}

	// プレイヤーと遠距離敵本体の接触判定
	if (player_ != nullptr && !player_->IsDead())
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		for (ArrowEnemy* arrowEnemy : arrowEnemies_)
		{
			if (arrowEnemy == nullptr || arrowEnemy->IsDead())
			{
				continue;
			}

			const Vector3 enemyPosition = arrowEnemy->GetWorldPosition();

			if (!IsPlayerEnemyCollision(playerPosition, enemyPosition))
			{
				continue;
			}

			player_->OnHit(1);

			if (player_->IsDead())
			{
				break;
			}
		}
	}

	// プレイヤーと盾敵本体の接触判定
	if (player_ != nullptr && !player_->IsDead()) 
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		for (ShieldEnemy* shieldEnemy : shieldEnemies_)
		{
			if (shieldEnemy == nullptr || shieldEnemy->IsDead()) 
			{
				continue;
			}

			// 突進中は既存の突進専用判定に任せる
			if (shieldEnemy->IsCharging())
			{
				continue;
			}

			const Vector3 enemyPosition = shieldEnemy->GetWorldPosition();

			if (!IsPlayerEnemyCollision(playerPosition, enemyPosition))
			{
				continue;
			}

			player_->OnHit(1);

			if (player_->IsDead())
			{
				break;
			}
		}
	}

	// プレイヤーとダメージ床の当たり判定
	if (player_ != nullptr && !player_->IsDead()) 
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		for (Spike* spike : spikes_)
		{
			if (spike == nullptr)
			{
				continue;
			}

			const Vector3 spikePosition = spike->GetWorldPosition();

			if (!IsPlayerSpikeCollision(playerPosition, spikePosition)) 
			{
				continue;
			}

			// ダメージ床に触れたので1ダメージ
			player_->OnHit(1);

			// 現在HP1なので、死亡したらこれ以上判定しない
			if (player_->IsDead())
			{
				break;
			}
		}
	}

	// 動く床の更新・プレイヤーの追従・当たり判定
	for (MoveBlock* moveBlock : moveBlocks_) 
	{
		if (moveBlock == nullptr)
		{
			continue;
		}

		bool wasPlayerOnMoveBlock = false;

		// MoveBlockが動く前に、
		// プレイヤーがその上に乗っていたか確認する
		if (player_ != nullptr && !player_->IsDead())
		{
			const Vector3 playerPosition = player_->GetWorldPosition();

			const Vector3 moveBlockPosition = moveBlock->GetWorldPosition();

			wasPlayerOnMoveBlock = IsPlayerOnMoveBlock(playerPosition, moveBlockPosition);
		}

		// MoveBlockを移動
		moveBlock->Update();

		if (player_ != nullptr && !player_->IsDead())
		{
			// 移動前に乗っていた場合、
			// MoveBlockが動いた分だけプレイヤーも移動
			if (wasPlayerOnMoveBlock)
			{
				player_->MoveBy(moveBlock->GetMoveAmount());
			}

			// 最後にMoveBlockとの衝突を解決
			player_->ResolveMoveBlockCollision(moveBlock->GetWorldPosition(), moveBlock->GetMoveAmount());
		}
	}

	// プレイヤーとシャッタードアの当たり判定
	if (player_ != nullptr && !player_->IsDead())
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		for (ShutterDoor* shutterDoor : shutterDoors_)
		{
			if (shutterDoor == nullptr) 
			{
				continue;
			}

			// 完全に開いたドアには当たり判定を付けない
			if (shutterDoor->IsHidden()) 
			{
				continue;
			}

			const Vector3 doorPosition = shutterDoor->GetWorldPosition();

			if (!IsPlayerShutterDoorCollision(playerPosition, doorPosition)) 
			{
				continue;
			}

			// プレイヤーがドアの左側にいるなら左へ押し戻す
			if (playerPosition.x < doorPosition.x) 
			{
				player_->PushBack(-0.1f);
			}
			// プレイヤーがドアの右側にいるなら右へ押し戻す
			else 
			{
				player_->PushBack(0.1f);
			}
		}
	}

	// 盾敵とプレイヤーの当たり判定
	if (player_ != nullptr && !player_->IsDead())
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		for (ShieldEnemy* shieldEnemy : shieldEnemies_)
		{
			if (shieldEnemy == nullptr || shieldEnemy->IsDead())
			{
				continue;
			}

			// 突進中だけ判定する
			if (!shieldEnemy->IsCharging())
			{
				continue;
			}

			const Vector3 enemyPosition = shieldEnemy->GetWorldPosition();

			if (!IsPlayerEnemyCollision(playerPosition, enemyPosition))
			{
				continue;
			}

			// 盾敵がプレイヤーの正面にいるか
			const bool isEnemyInFront = IsEnemyInFront(playerPosition, enemyPosition, player_->GetLRDirection());

			// 正面を向いてガードしているか
			const bool canGuard = player_->IsGuarding() && isEnemyInFront;

			if (canGuard) 
			{
				// ガード成功
				// 敵とは反対方向へプレイヤーをノックバック
				if (enemyPosition.x >= playerPosition.x) 
				{
					// 敵が右側にいるので、プレイヤーを左へ押す
					player_->PushBack(-kChargePlayerPushBackDistance);
				}
				else 
				{
					// 敵が左側にいるので、プレイヤーを右へ押す
					player_->PushBack(kChargePlayerPushBackDistance);
				}

				// ガードされたので突進終了
				shieldEnemy->StopCharge();
			}
			else 
			{
				// ガードできなかったので1ダメージ
				player_->OnHit(1);

				// プレイヤーに命中したので突進終了
				shieldEnemy->StopCharge();
			}
		}
	}

	// 遠距離敵からの射撃要求を処理
	for (ArrowEnemy* arrowEnemy : arrowEnemies_)
	{
		if (arrowEnemy == nullptr || arrowEnemy->IsDead())
		{
			continue;
		}

		if (!arrowEnemy->IsShootRequested())
		{
			continue;
		}

		// 弾の発射位置
		const Vector3 spawnPosition = arrowEnemy->GetBulletSpawnPosition();

		// 弾の速度
		Vector3 bulletVelocity{};

		constexpr float kArrowBulletSpeed = 0.15f;

		if (arrowEnemy->GetDirection() == ArrowEnemyDirection::kRight)
		{
			bulletVelocity = {kArrowBulletSpeed, 0.0f, 0.0f};
		}
		else 
		{
			bulletVelocity = 
			{
				-kArrowBulletSpeed, 0.0f, 0.0f
			};
		}

		// 弾生成
		ArrowBullet* bullet = new ArrowBullet();

		bullet->Initialize(arrowBulletModel_, &camera_, spawnPosition, bulletVelocity);

		bullet->SetMapChipField(mapChipField_);

		arrowBullets_.push_back(bullet);

		// 発射要求を消費
		arrowEnemy->ClearShootRequest();
	}

	// 遠距離敵の弾を更新
	for (ArrowBullet* bullet : arrowBullets_)
	{
		if (bullet == nullptr) 
		{
			continue;
		}

		bullet->Update();
	}

	// ArrowBulletとプレイヤーの当たり判定
	if (player_ != nullptr && !player_->IsDead())
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		for (ArrowBullet* bullet : arrowBullets_) 
		{
			if (bullet == nullptr || bullet->IsDead())
			{
				continue;
			}

			const Vector3 bulletPosition = bullet->GetWorldPosition();

			if (!IsPlayerArrowBulletCollision(playerPosition, bulletPosition)) 
			{
				continue;
			}

			// 弾がプレイヤーの正面から来たか
			const bool isBulletInFront = IsEnemyInFront(playerPosition, bulletPosition, player_->GetLRDirection());

			// 正面を向いてガードしていれば防御成功
			const bool canGuard = player_->IsGuarding() && isBulletInFront;

			if (!canGuard)
			{
				// ガードできなかったので1ダメージ
				player_->OnHit(1);
			}

			// 命中・ガードのどちらでも弾は消滅
			bullet->SetDead();
		}
	}

	// 消滅した弾を削除
	arrowBullets_.erase(
	std::remove_if( arrowBullets_.begin(), arrowBullets_.end(),[](ArrowBullet* bullet)
	{
		        if (bullet == nullptr) 
				{
			        return true;
		        }

		        if (bullet->IsDead())
				{
			        delete bullet;
			        return true;
		        }

		        return false;
	}),
	    arrowBullets_.end());

	// プレイヤーと敵の接触判定
	if (player_ != nullptr && !player_->IsDead()) 
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		for (Enemy* enemy : enemies_)
		{
			if (enemy == nullptr || enemy->IsDead())
			{
				continue;
			}

			// 剣の攻撃範囲に入っていなければ次の敵へ
			if (!enemy->IsPositionInAttackRange(playerPosition)) 
			{
				continue;
			}

			const Vector3 enemyPosition = enemy->GetWorldPosition();

			// 敵がプレイヤーの正面側にいるか
			const bool isEnemyInFront = IsEnemyInFront(playerPosition, enemyPosition, player_->GetLRDirection());

			const bool canGuard = player_->IsGuarding() && isEnemyInFront;

			if (canGuard) 
			{
				// ガード成功時は敵をプレイヤーから遠ざける
				if (enemyPosition.x >= playerPosition.x) 
				{
					enemy->PushBack(kGuardPushBackDistance);
				}
				else
				{
					enemy->PushBack(-kGuardPushBackDistance);
				}
			}
			else 
			{
				// 剣攻撃で1ダメージ
				player_->OnHit(1);
			}

			// ガード・ダメージのどちらでも、
			// 今回の斬撃は処理済みにする
			enemy->SetAttackHit();
		}
	}

	// ボスの剣攻撃とプレイヤーの当たり判定
	if (boss_ != nullptr && !boss_->IsDead() && player_ != nullptr && !player_->IsDead()) 
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		if (boss_->IsPositionInAttackRange(playerPosition)) 
		{
			const Vector3 bossPosition = boss_->GetWorldPosition();

			const bool isBossInFront = IsEnemyInFront(playerPosition, bossPosition, player_->GetLRDirection());

			const bool canGuard = player_->IsGuarding() && isBossInFront;

			if (!canGuard) 
			{
				player_->OnHit(1);
			}

			// ガードでも被弾でも、
			// 今回の剣攻撃は処理済み
			boss_->SetAttackHit();
		}
	}

	// ボスの回転攻撃とプレイヤーの当たり判定
	if (boss_ != nullptr && !boss_->IsDead() && player_ != nullptr && !player_->IsDead())
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		if (boss_->IsPositionInSpinAttackRange(playerPosition)) 
		{
			// 回転攻撃は全方向攻撃なので、
			// プレイヤーがどちらを向いていてもガード不可
			player_->OnHit(1);

			boss_->SetAttackHit();
		}
	}

	// ボス本体とプレイヤーの接触判定
	if (boss_ != nullptr && !boss_->IsDead() && player_ != nullptr && !player_->IsDead())
	{
		// 突進中は既存の突進専用判定に任せる
		if (!boss_->IsCharging())
		{
			const Vector3 playerPosition = player_->GetWorldPosition();
			const Vector3 bossPosition = boss_->GetWorldPosition();

			if (IsPlayerBossCollision(playerPosition, bossPosition))
			{
				player_->OnHit(1);
			}
		}
	}

	// ボスの射撃要求を処理
	if (boss_ != nullptr && !boss_->IsDead())
	{
		if (boss_->IsShootRequested())
		{
			const Vector3 spawnPosition = boss_->GetBulletSpawnPosition();

			Vector3 bulletVelocity{};

			// 通常の遠距離敵より少し速め
			constexpr float kBossArrowBulletSpeed = 0.18f;

			if (boss_->GetDirection() == BossDirection::kRight)
			{
				bulletVelocity = {kBossArrowBulletSpeed, 0.0f, 0.0f};
			}
			else 
			{
				bulletVelocity = {-kBossArrowBulletSpeed, 0.0f, 0.0f};
			}

			ArrowBullet* bullet = new ArrowBullet();

			bullet->Initialize(arrowBulletModel_, &camera_, spawnPosition, bulletVelocity);

			bullet->SetMapChipField(mapChipField_);

			arrowBullets_.push_back(bullet);

			// 発射要求を消費
			boss_->ClearShootRequest();
		}
	}

	// ボスの突進攻撃とプレイヤーの当たり判定
	if (boss_ != nullptr && !boss_->IsDead() && boss_->IsCharging() && player_ != nullptr && !player_->IsDead())
	{
		const Vector3 playerPosition = player_->GetWorldPosition();

		const Vector3 bossPosition = boss_->GetWorldPosition();

		// ボスは大きいので専用判定
		constexpr float kPlayerHalfWidth = 0.4f;
		constexpr float kPlayerHalfHeight = 0.4f;

		constexpr float kBossHalfWidth = 1.0f;
		constexpr float kBossHalfHeight = 1.0f;

		const float distanceX = std::abs(playerPosition.x - bossPosition.x);

		const float distanceY = std::abs(playerPosition.y - bossPosition.y);

		const bool isCollision = distanceX <= kPlayerHalfWidth + kBossHalfWidth && distanceY <= kPlayerHalfHeight + kBossHalfHeight;

		if (isCollision)
		{
			const bool isBossInFront = IsEnemyInFront(playerPosition, bossPosition, player_->GetLRDirection());

			const bool canGuard = player_->IsGuarding() && isBossInFront;

			if (canGuard) 
			{
				// 盾敵と同じく、
				// プレイヤーを反対方向へ押す
				if (bossPosition.x >= playerPosition.x) 
				{
					player_->PushBack(-kChargePlayerPushBackDistance);
				} 
				else
				{
					player_->PushBack(kChargePlayerPushBackDistance);
				}
			}
			else
			{
				player_->OnHit(1);
			}

			// ガード・命中どちらでも突進終了
			boss_->StopCharge();
		}
	}

	// 現在のZキー入力
	const bool isThrowKeyPressed = input->PushKey(DIK_Z);

	// 今押されていて、前フレームでは押されていなかった
	if (isThrowKeyPressed && !wasThrowKeyPressed_ && !player_->IsDead() && !player_->IsGuarding() && !boomerang_->IsThrown())
	{
		// 新しく盾を投げるので、命中記録をリセット
		hitEnemiesOutbound_.clear();
		hitEnemiesReturn_.clear();

		hitArrowEnemiesOutbound_.clear();
		hitArrowEnemiesReturn_.clear();

		hitShieldEnemiesOutbound_.clear();
		hitShieldEnemiesReturn_.clear();

		hitBossOutbound_ = false;
		hitBossReturn_ = false;

		boomerang_->Throw();
	}

	// 現在の入力を次フレーム用に保存
	wasThrowKeyPressed_ = isThrowKeyPressed;

	// ブーメラン更新
	boomerang_->Update();

	// 行きのブーメランとMoveBlockの当たり判定
	if (boomerang_->GetPhase() == Boomerang::Phase::kOutbound) 
	{
		const Vector3 boomerangPosition = boomerang_->GetWorldPosition();

		for (MoveBlock* moveBlock : moveBlocks_) 
		{
			if (moveBlock == nullptr) 
			{
				continue;
			}

			const Vector3 moveBlockPosition = moveBlock->GetWorldPosition();

			if (IsBoomerangMoveBlockCollision(boomerangPosition, moveBlockPosition)) 
			{
				// MoveBlockに当たったので帰還開始
				boomerang_->StartReturn();

				break;
			}
		}
	}

	// 投げたブーメランと敵の当たり判定
	if (boomerang_->IsThrown()) 
	{
		const Vector3 boomerangPosition = boomerang_->GetWorldPosition();

		const Boomerang::Phase phase = boomerang_->GetPhase();

		for (Enemy* enemy : enemies_)
		{
			if (enemy == nullptr || enemy->IsDead()) 
			{
				continue;
			}

			const Vector3 enemyPosition = enemy->GetWorldPosition();

			if (!IsEnemyBoomerangCollision(enemyPosition, boomerangPosition))
			{
				continue;
			}

			// 行きでの命中
			if (phase == Boomerang::Phase::kOutbound)
			{
				const bool hasAlreadyHit = hitEnemiesOutbound_.find(enemy) != hitEnemiesOutbound_.end();

				if (!hasAlreadyHit)
				{
					enemy->OnHit(1);
					hitEnemiesOutbound_.insert(enemy);
				}
			}
			// 帰りでの命中
			else if (phase == Boomerang::Phase::kReturn) 
			{
				const bool hasAlreadyHit = hitEnemiesReturn_.find(enemy) != hitEnemiesReturn_.end();

				if (!hasAlreadyHit)
				{
					enemy->OnHit(1);
					hitEnemiesReturn_.insert(enemy);
				}
			}
		}

		// 遠距離敵との当たり判定
		for (ArrowEnemy* arrowEnemy : arrowEnemies_)
		{
			if (arrowEnemy == nullptr || arrowEnemy->IsDead())
			{
				continue;
			}

			const Vector3 arrowEnemyPosition = arrowEnemy->GetWorldPosition();

			if (!IsEnemyBoomerangCollision(arrowEnemyPosition, boomerangPosition))
			{
				continue;
			}

			// 行きでの命中
			if (phase == Boomerang::Phase::kOutbound)
			{
				const bool hasAlreadyHit = hitArrowEnemiesOutbound_.find(arrowEnemy) != hitArrowEnemiesOutbound_.end();

				if (!hasAlreadyHit)
				{
					arrowEnemy->OnHit(1);

					hitArrowEnemiesOutbound_.insert(arrowEnemy);
				}
			}
			// 帰りでの命中
			else if (phase == Boomerang::Phase::kReturn)
			{
				const bool hasAlreadyHit = hitArrowEnemiesReturn_.find(arrowEnemy) != hitArrowEnemiesReturn_.end();

				if (!hasAlreadyHit) 
				{
					arrowEnemy->OnHit(1);

					hitArrowEnemiesReturn_.insert(arrowEnemy);
				}
			}
		}

		for (ShieldEnemy* shieldEnemy : shieldEnemies_)
		{
			if (shieldEnemy == nullptr || shieldEnemy->IsDead()) 
			{
				continue;
			}

			const Vector3 shieldEnemyPosition = shieldEnemy->GetWorldPosition();

			if (!IsEnemyBoomerangCollision(shieldEnemyPosition, boomerangPosition)) 
			{
				continue;
			}

			// 行きでの命中
			if (phase == Boomerang::Phase::kOutbound)
			{
				const bool hasAlreadyHit = hitShieldEnemiesOutbound_.find(shieldEnemy) != hitShieldEnemiesOutbound_.end();

				if (!hasAlreadyHit) 
				{
					if (shieldEnemy->IsPositionInFront(boomerangPosition)) 
					{
						// 正面なので盾で防御
						shieldEnemy->OnGuard();

						// ブーメランもここで止める
						boomerang_->StartReturn();

					}
					else
					{
						// 背後ならダメージ
						shieldEnemy->OnHit(1);
					}

					hitShieldEnemiesOutbound_.insert(shieldEnemy);
				}
			}
			// 帰りでの命中
			else if (phase == Boomerang::Phase::kReturn)
			{
				const bool hasAlreadyHit = hitShieldEnemiesReturn_.find(shieldEnemy) != hitShieldEnemiesReturn_.end();

				if (!hasAlreadyHit)
				{
					if (!shieldEnemy->IsPositionInFront(boomerangPosition)) 
					{
						shieldEnemy->OnHit(1);
					}

					hitShieldEnemiesReturn_.insert(shieldEnemy);
				}
			}
		}

		// ボスとの当たり判定
		if (boss_ != nullptr && !boss_->IsDead()) 
		{
			const Vector3 bossPosition = boss_->GetWorldPosition();

			if (IsBossBoomerangCollision(bossPosition, boomerangPosition))
			{
				// 行き
				if (phase == Boomerang::Phase::kOutbound) 
				{
					if (!hitBossOutbound_)
					{
						if (boss_->IsPositionInFront(boomerangPosition)) 
						{
							// 正面攻撃は無効化

							// ブーメランを強制的に帰還させる
							boomerang_->StartReturn();
						}
						else 
						{
							// 背後なのでダメージ
							boss_->OnHit(1);
						}

						hitBossOutbound_ = true;
					}
				}

				// 帰り
				else if (phase == Boomerang::Phase::kReturn)
				{
					if (!hitBossReturn_) 
					{
						// 帰りも背後からならダメージ
						if (!boss_->IsPositionInFront(boomerangPosition)) 
						{
							boss_->OnHit(1);
						}

						// 正面なら何も起こらない
						hitBossReturn_ = true;
					}
				}
			}
		}

		// スイッチとの当たり判定
		for (Switch* switchObject : switches_) 
		{
			if (switchObject == nullptr)
			{
				continue;
			}

			// すでにONなら再度処理しない
			if (switchObject->IsOn())
			{
				continue;
			}

			const Vector3 switchPosition = switchObject->GetWorldPosition();

			if (!IsSwitchBoomerangCollision(switchPosition, boomerangPosition))
			{
				continue;
			}

			// スイッチをONにする
			switchObject->OnSwitch();
			
		}

	}

	// 天球更新
	skyDome_->Update();

	backGroundWall_->Update();

	// プレイヤーが死亡していたらゲームオーバー
	if (player_ != nullptr && player_->IsDead())
	{
		isGameOver_ = true;
	}

}

void GameScene::Draw()
{
	// 天球
	skyDome_->Draw();

	backGroundWall_->Draw();

	// プレイヤー
	player_->Draw();

	// ブーメラン
	boomerang_->Draw();

	for (Enemy* enemy : enemies_)
	{
		if (enemy == nullptr)
		{
			continue;
		}

		enemy->Draw();
	}

	// 遠距離敵
	for (ArrowEnemy* arrowEnemy : arrowEnemies_) 
	{
		if (arrowEnemy == nullptr)
		{
			continue;
		}

		arrowEnemy->Draw();
	}

	// 盾敵
	for (ShieldEnemy* shieldEnemy : shieldEnemies_) 
	{
		if (shieldEnemy == nullptr) 
		{
			continue;
		}

		shieldEnemy->Draw();
	}

	// 遠距離敵の弾
	for (ArrowBullet* bullet : arrowBullets_) 
	{
		if (bullet == nullptr)
		{
			continue;
		}

		bullet->Draw();
	}

	// スイッチ
	for (Switch* switchObject : switches_) 
	{
		if (switchObject == nullptr) 
		{
			continue;
		}

		switchObject->Draw();
	}

	// シャッタードア
	for (ShutterDoor* shutterDoor : shutterDoors_)
	{
		if (shutterDoor == nullptr)
		{
			continue;
		}

		shutterDoor->Draw();
	}

	// 動く床
	for (MoveBlock* moveBlock : moveBlocks_) 
	{
		if (moveBlock == nullptr)
		{
			continue;
		}

		moveBlock->Draw();
	}

	// 横レール
	for (WorldTransform* worldTransform : worldTransformBesideRails_)
	{
		if (worldTransform == nullptr)
		{
			continue;
		}

		besideRailModel_->Draw(*worldTransform, camera_);
	}

	// 縦レール
	for (WorldTransform* worldTransform : worldTransformVerticalRails_)
	{
		if (worldTransform == nullptr)
		{
			continue;
		}

		verticalRailModel_->Draw(*worldTransform, camera_);
	}

	// ダメージ床
	for (Spike* spike : spikes_)
	{
		if (spike == nullptr)
		{
			continue;
		}

		spike->Draw();
	}

	// ボス
	if (boss_ != nullptr) 
	{
		boss_->Draw();
	}

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
	delete cameraController_;
	cameraController_ = nullptr;

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

	for (Enemy*& enemy : enemies_)
	{
		delete enemy;
		enemy = nullptr;
	}

	enemies_.clear();
	hitEnemiesOutbound_.clear();
	hitEnemiesReturn_.clear();

	delete enemyModel_;
	enemyModel_ = nullptr;

	delete swordModel_;
	swordModel_ = nullptr;

	for (ArrowEnemy*& arrowEnemy : arrowEnemies_)
	{
		delete arrowEnemy;
		arrowEnemy = nullptr;
	}

	arrowEnemies_.clear();

	for (ArrowBullet*& bullet : arrowBullets_)
	{
		delete bullet;
		bullet = nullptr;
	}

	arrowBullets_.clear();

	delete arrowEnemyModel_;
	arrowEnemyModel_ = nullptr;

	delete arrowModel_;
	arrowModel_ = nullptr;

	delete arrowBulletModel_;
	arrowBulletModel_ = nullptr;

	for (ShieldEnemy*& shieldEnemy : shieldEnemies_) 
	{
		delete shieldEnemy;
		shieldEnemy = nullptr;
	}

	shieldEnemies_.clear();

	hitShieldEnemiesOutbound_.clear();
	hitShieldEnemiesReturn_.clear();

	delete shieldEnemyModel_;
	shieldEnemyModel_ = nullptr;

	delete enemyShieldModel_;
	enemyShieldModel_ = nullptr;

	for (Switch*& switchObject : switches_) 
	{
		delete switchObject;
		switchObject = nullptr;
	}

	switches_.clear();

	for (ShutterDoor*& shutterDoor : shutterDoors_)
	{
		delete shutterDoor;
		shutterDoor = nullptr;
	}

	shutterDoors_.clear();

	delete switchModel_;
	switchModel_ = nullptr;

	delete shutterDoorModel_;
	shutterDoorModel_ = nullptr;

	for (MoveBlock*& moveBlock : moveBlocks_) 
	{
		delete moveBlock;
		moveBlock = nullptr;
	}

	moveBlocks_.clear();

	delete moveBlockModel_;
	moveBlockModel_ = nullptr;

	for (WorldTransform*& worldTransform : worldTransformBesideRails_)
	{
		delete worldTransform;
		worldTransform = nullptr;
	}

	worldTransformBesideRails_.clear();

	delete besideRailModel_;
	besideRailModel_ = nullptr;

	for (WorldTransform*& worldTransform : worldTransformVerticalRails_)
	{
		delete worldTransform;
		worldTransform = nullptr;
	}

	worldTransformVerticalRails_.clear();

	delete verticalRailModel_;
	verticalRailModel_ = nullptr;

	for (Spike*& spike : spikes_) 
	{
		delete spike;
		spike = nullptr;
	}

	spikes_.clear();

	delete spikeBlockModel_;
	spikeBlockModel_ = nullptr;

	delete boss_;
	boss_ = nullptr;

	delete bossModel_;
	bossModel_ = nullptr;

	delete bossSwordModel_;
	bossSwordModel_ = nullptr;

	delete crossbowModel_;
	crossbowModel_ = nullptr;

	delete backGroundWall_;

	backGroundWall_ = nullptr;

	delete backGroundWallModel_;

	backGroundWallModel_ = nullptr;

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