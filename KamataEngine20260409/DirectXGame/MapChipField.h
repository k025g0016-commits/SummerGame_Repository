#pragma once
#include "KamataEngine.h"
#include <cstdint>
#include <string>
#include <vector>

// マップチップの種類
enum class MapChipType 
{
	kBlank,        // 空白
	kBlock,        // B0
	kSpikeBlock,   // B1
	kPlayer,       // P0
	kEnemy,        // E0
	kArrowEnemy,   // E1
	kShieldEnemy,  // E2
	kBoss,         // E3
	kBossArea,     // A0
	kSwitch,       // G0
	kShutterDoor,  // G1
	kMoveBlock,    // G2
	kBesideRail,   // R0
	kVerticalRail, // R1

	// チュートリアル
	kMoveTutorial,    // T0
	kJumpTutorial,    // T1
	kAttackTutorial,  // T2
	kDefenceTutorial, // T3
};

// マップチップの番号
struct MapChipIndexSet 
{
	int32_t xIndex;
	int32_t yIndex;
};

// マップチップ1個分の矩形
struct MapChipRect
{
	float left;
	float right;
	float bottom;
	float top;
};

// グループを持つマップチップの情報
struct MapChipGroupData 
{
	KamataEngine::Vector3 position;
	int32_t groupId;
};

class MapChipField 
{
public:
	// CSVファイルの読み込み
	void LoadMapChipCsv(const std::string& filePath);

	// 指定位置のマップチップを取得
	MapChipType GetMapChipTypeByIndex(int32_t xIndex, int32_t yIndex) const;

	// 指定したマップチップのワールド座標を取得
	KamataEngine::Vector3 GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const;

	// 指定したマップチップの矩形を取得
	MapChipRect GetRectByIndex(int32_t xIndex, int32_t yIndex) const;

	// ワールド座標からマップチップ番号を取得
	MapChipIndexSet GetMapChipIndexSetByPosition(const KamataEngine::Vector3& position) const;

	// プレイヤーの初期位置を取得
	KamataEngine::Vector3 GetPlayerPosition() const;

	// 剣敵E0の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetEnemyPositions() const;

	// 遠距離敵E1の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetArrowEnemyPositions() const;

	// 横のマップチップ数
	uint32_t GetNumBlockHorizontal() const;

	// 縦のマップチップ数
	uint32_t GetNumBlockVertical() const;

	// 盾敵E2の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetShieldEnemyPositions() const;

	// スイッチG0の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetSwitchPositions() const;

	// シャッタードアG1の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetShutterDoorPositions() const;

	// グループ番号付きSwitch情報を取得
	std::vector<MapChipGroupData> GetSwitchGroupData() const;

	// グループ番号付きShutterDoor情報を取得
	std::vector<MapChipGroupData> GetShutterDoorGroupData() const;

	// 動く床G2の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetMoveBlockPositions() const;

	// 横レールR0の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetBesideRailPositions() const;

	// 縦レールR1の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetVerticalRailPositions() const;

	// ダメージ床B1の初期位置をすべて取得
	std::vector<KamataEngine::Vector3> GetSpikeBlockPositions() const;

	// ボスE3の初期位置を取得
	KamataEngine::Vector3 GetBossPosition() const;

	// ボスエリアA0の位置を取得
	std::vector<KamataEngine::Vector3> GetBossAreaPositions() const;

	// ボスエリアの開始位置を取得
	KamataEngine::Vector3 GetBossAreaPosition() const;

	std::vector<KamataEngine::Vector3> GetMoveTutorialPositions() const;
	std::vector<KamataEngine::Vector3> GetJumpTutorialPositions() const;
	std::vector<KamataEngine::Vector3> GetAttackTutorialPositions() const;
	std::vector<KamataEngine::Vector3> GetDefenceTutorialPositions() const;

	bool HasBoss() const;

private:
	// 文字列からマップチップの種類へ変換
	MapChipType ParseMapChipType(const std::string& word) const;

	// マップチップ文字列からグループ番号を取得
	int32_t ParseGroupId(const std::string& word) const;

private:
	// マップチップデータ
	std::vector<std::vector<MapChipType>> mapChipData_;

	// 各マップチップのグループ番号
	std::vector<std::vector<int32_t>> mapChipGroupIds_;

	// 1ブロックの幅と高さ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

};