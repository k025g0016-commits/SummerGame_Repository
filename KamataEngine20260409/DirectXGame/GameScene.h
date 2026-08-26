#pragma once
#include "KamataEngine.h"
#include <unordered_set>
#include <vector>
#include "CameraController.h"
#include "Enemy.h"
#include "BackGroundWall.h"
#include <unordered_map>

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
class Tutorial;
class RingEffect;
class SparkParticle;
class DeathParticle;

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

	// タイトルへ戻る要求が出ているか
	bool IsReturnTitleRequested() const 
	{
		return isReturnTitleRequested_;
	}

	// ポーズ時の暗転描画
	void DrawPauseDark();

	// ポーズメニュー描画
	void DrawPauseMenu();

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

	// チュートリアルモデル
	KamataEngine::Model* tutorialPlateModel_ = nullptr;
	KamataEngine::Model* moveTutorialModel_ = nullptr;
	KamataEngine::Model* jumpTutorialModel_ = nullptr;
	KamataEngine::Model* attackTutorialModel_ = nullptr;
	KamataEngine::Model* defenceTutorialModel_ = nullptr;
	KamataEngine::Model* pauseTutorialModel_ = nullptr;

	// ポーズ画面モデル
	KamataEngine::Model* selectSwitchPlateModel_ = nullptr;
	KamataEngine::Model* returnToGameModel_ = nullptr;
	KamataEngine::Model* returnToTitleModel_ = nullptr;

	// リングエフェクトモデル
	KamataEngine::Model* ringEffectModel_ = nullptr;

	// 火花パーティクルモデル
	KamataEngine::Model* sparkParticleModel_ = nullptr;

	// 死亡パーティクルモデル
	KamataEngine::Model* deathParticleModel_ = nullptr;

	// 死亡パーティクル
	DeathParticle* deathParticle_ = nullptr;

	// 死亡演出を開始したか
	bool isDeathEffectStarted_ = false;

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

	// ボスエリアの境界
	float bossAreaLeft_ = 0.0f;
	float bossAreaRight_ = 0.0f;
	float bossAreaBottom_ = 0.0f;
	float bossAreaTop_ = 0.0f;

	// ボスエリアに入ったか
	bool isBossAreaEntered_ = false;

	// 通常時のカメラ移動範囲
	CameraController::Rect normalCameraArea_{};

	// ゲームオーバーフラグ
	bool isGameOver_ = false;

	BackGroundWall* backGroundWall_ = nullptr;

	bool isGameClear_ = false;

	// チュートリアル
	std::vector<Tutorial*> tutorials_;

	RingEffect* ringEffect_ = nullptr;

	// ガード成功時のリングエフェクト生成
	void CreateGuardRingEffect();

	// 指定位置にリングエフェクトを生成
	void CreateRingEffect(const KamataEngine::Vector3& position);

    // 指定位置に火花パーティクルを生成
	void CreateSparkParticles(const KamataEngine::Vector3& position);

	// 火花パーティクル
	std::vector<SparkParticle*> sparkParticles_;

	// 通常BGM
	uint32_t gamePlayBGMHandle_ = 0;

	// 現在再生している通常BGM
	uint32_t gamePlayBGMVoiceHandle_ = 0;

	// ボスBGM
	uint32_t bossBGMHandle_ = 0;

	// 現在再生しているボスBGM
	uint32_t bossBGMVoiceHandle_ = 0;

	// クリア移行中か
	bool isClearTransition_ = false;

	// ボスBGMフェード時間
	float bossBGMFadeTimer_ = 0.0f;

	// フェード時間
	static inline const float kBossBGMFadeDuration = 1.0f;

	// ボスBGM通常音量
	static inline const float kBossBGMVolume = 0.5f;

	// ガードSE
	uint32_t guardSEHandle_ = 0;

	// 発射SE
	uint32_t shotSEHandle_ = 0;

	// 剣攻撃SE
	uint32_t swordAttackSEHandle_ = 0;

	// 突進SE
	uint32_t dashSEHandle_ = 0;

	// 盾敵ごとの突進SE再生ハンドル
	std::unordered_map<ShieldEnemy*, uint32_t> shieldEnemyDashVoiceHandles_;

	// ボスの突進SE再生ハンドル
	uint32_t bossDashVoiceHandle_ = 0;

	// ボスの突進SEを再生中か
	bool isBossDashSEPlaying_ = false;

	// スイッチSE
	uint32_t switchSEHandle_ = 0;

	// プレイヤー死亡SE
	uint32_t playerDeathSEHandle_ = 0;

	// 敵ダメージSE
	uint32_t enemyDamageSEHandle_ = 0;

	// 敵死亡SE
	uint32_t enemyDeathSEHandle_ = 0;

	// ゲーム中SEの再生ハンドル
	std::vector<uint32_t> gameSEVoiceHandles_;

	// PlayerDeath以外のゲーム中SEを停止
	void StopGameSEs();

	// ゲーム中SEを再生
	void PlayGameSE(uint32_t soundHandle, bool loopFlag = false, float volume = 0.5f);

	// ジャンプSE
	uint32_t jumpSEHandle_ = 0;

	// ポーズメニューカーソルSE
	uint32_t cursorSEHandle_ = 0;

	// ポーズメニュー決定SE
	uint32_t decisionSEHandle_ = 0;

	// ポーズ中か
	bool isPaused_ = false;

	// 前フレームでESCキーが押されていたか
	bool wasPauseKeyPressed_ = false;

	// タイトルへ戻る要求
	bool isReturnTitleRequested_ = false;

	// ポーズメニューの選択項目
	enum class PauseMenuItem
	{
		kReturnToGame,
		kReturnToTitle,
	};

	// 現在選択中の項目
	PauseMenuItem pauseMenuItem_ = PauseMenuItem::kReturnToGame;

	// ポーズメニュー入力
	bool wasPauseUpKeyPressed_ = false;
	bool wasPauseDownKeyPressed_ = false;
	bool wasPauseDecideKeyPressed_ = false;

	// ポーズ画面のワールド変換
	KamataEngine::WorldTransform returnToGamePlateTransform_;
	KamataEngine::WorldTransform returnToTitlePlateTransform_;
	KamataEngine::WorldTransform returnToGameTransform_;
	KamataEngine::WorldTransform returnToTitleTransform_;

	// ポーズ画面専用カメラ
	KamataEngine::Camera pauseCamera_;

	// ポーズメニュー文字の点滅
	float pauseBlinkTimer_ = 0.0f;
	bool isPauseSelectedTextVisible_ = true;

	// 点滅間隔
	static inline const float kPauseBlinkInterval = 0.4f;

	// タイトルへ戻る時のBGMフェード中か
	bool isReturnTitleBGMFadingOut_ = false;

	// タイトルへ戻る時のBGMフェード時間
	float returnTitleBGMFadeTimer_ = 0.0f;

	// フェード時間
	static inline const float kReturnTitleBGMFadeDuration = 0.5f;

	// ポーズ時の暗転用スプライト
	KamataEngine::Sprite* pauseDarkSprite_ = nullptr;

};