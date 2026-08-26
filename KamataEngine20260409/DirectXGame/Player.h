#pragma once
#include "KamataEngine.h"
#include <array>

class MapChipField;

class Player 
{
public:
	enum class LRDirection 
	{ 
		kRight,
		kLeft 
	};

public:
	// 初期化
	void Initialize(KamataEngine::Model* model, const KamataEngine::Camera* camera, const KamataEngine::Vector3& position);

	// 更新
	void Update();

	// 描画
	void Draw();

	// ワールド座標取得
	KamataEngine::Vector3 GetWorldPosition() const;

	// 盾の所持位置を取得
	KamataEngine::Vector3 GetShieldPosition() const;

	// 向き取得
	LRDirection GetLRDirection() const
	{
		return lrDirection_;
	}

	// マップチップフィールドを設定
	void SetMapChipField(MapChipField* mapChipField) 
	{
		mapChipField_ = mapChipField; 
	}

	// 盾を構えているか
	bool IsGuarding() const
	{
		return isGuarding_; 
	}

	// ダメージを受ける
	void OnHit(int damage);

	// 死亡しているか
	bool IsDead() const 
	{ 
		return isDead_; 
	}

	// 現在のHPを取得
	int GetHP() const 
	{
		return hp_;
	}

	void PushBack(float moveX);

	// 外部から移動量を加える
	void MoveBy(const KamataEngine::Vector3& move);

	// 動く床の上に着地させる
	void LandOnMoveBlock(float moveBlockTop);

    // 動く床との衝突を解決
	void ResolveMoveBlockCollision(const KamataEngine::Vector3& moveBlockPosition, const KamataEngine::Vector3& moveBlockMoveAmount);

	// ジャンプSE再生要求があるか
	bool IsJumpSERequested() const 
	{ 
		return isJumpSERequested_; 
	}

	// ジャンプSE再生要求を解除
	void ClearJumpSERequest() 
	{
		isJumpSERequested_ = false;
	}

private:
	// マップ衝突判定の結果
	struct CollisionMapInfo 
	{
		// 天井へ衝突したか
		bool ceiling = false;

		// 地面へ着地したか
		bool landing = false;

		// 壁へ衝突したか
		bool hitWall = false;

		// 衝突を考慮した移動量
		KamataEngine::Vector3 move = {};
	};

	// プレイヤーの四隅
	enum Corner
	{
		kRightBottom, // 右下
		kLeftBottom,  // 左下
		kRightTop,    // 右上
		kLeftTop,     // 左上

		kNumCorner
	};

private:
	// 移動入力
	void InputMove();

	// マップとの衝突判定
	void MapCollision(CollisionMapInfo& info);

	// 上方向の衝突判定
	void MapCollisionUp(CollisionMapInfo& info);

	// 下方向の衝突判定
	void MapCollisionDown(CollisionMapInfo& info);

	// 左方向の衝突判定
	void MapCollisionLeft(CollisionMapInfo& info);

	// 右方向の衝突判定
	void MapCollisionRight(CollisionMapInfo& info);

	// 衝突判定結果を反映する
	void ApplyCollisionResult(const CollisionMapInfo& info);

	// 接地状態を切り替える
	void UpdateOnGround(const CollisionMapInfo& info);

	// 盾構えの更新
	void UpdateGuard();

	// 指定した座標を中心とする四隅を計算
	std::array<KamataEngine::Vector3, kNumCorner> GetCornerPositions(const KamataEngine::Vector3& center) const;

private:
	KamataEngine::WorldTransform worldTransform_;

	KamataEngine::Model* model_ = nullptr;

	const KamataEngine::Camera* camera_ = nullptr;

	MapChipField* mapChipField_ = nullptr;

	LRDirection lrDirection_ = LRDirection::kRight;

	// 速度
	KamataEngine::Vector3 velocity_ = {};

	// 接地しているか
	bool onGround_ = false;

	// 盾を構えているか
	bool isGuarding_ = false;

	// 前フレームでSpaceキーが押されていたか
	bool wasJumpKeyPressed_ = false;

	// 左右移動速度
	static inline const float kMoveSpeed = 0.1f;

	// 重力
	static inline const float kGravity = 0.02f;

	// 最大落下速度
	static inline const float kLimitFallSpeed = 0.5f;

	// ジャンプ初速
	static inline const float kJumpSpeed = 0.35f;

	// プレイヤーの横幅
	static inline const float kWidth = 0.8f;

	// プレイヤーの縦幅
	static inline const float kHeight = 0.8f;

	// 接地確認時に足元を下へずらす量
	static inline const float kGroundCheckOffset = 0.05f;

	// 構え中の移動速度倍率
	static inline const float kGuardMoveRate = 0.5f;

	// 最大HP
	static inline const int kMaxHP = 1;

	// 現在のHP
	int hp_ = kMaxHP;

	// 死亡しているか
	bool isDead_ = false;

	// 描画専用ワールド変換
	KamataEngine::WorldTransform drawWorldTransform_;

	// ノックバックの横速度
	float knockBackVelocityX_ = 0.0f;

	// ノックバックの減速量
	static inline const float kKnockBackDeceleration = 0.03f;

	// 更新前のプレイヤー位置
	KamataEngine::Vector3 previousPosition_ = {};

	// ジャンプSE再生要求
	bool isJumpSERequested_ = false;

};