#pragma once
#include "KamataEngine.h"
#include <cstdint>
#include <string>
#include <vector>

// マップチップの種類
enum class MapChipType 
{
	kBlank, // 空白
	kBlock, // B0
	kPlayer // P0
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

	// 横のマップチップ数
	uint32_t GetNumBlockHorizontal() const;

	// 縦のマップチップ数
	uint32_t GetNumBlockVertical() const;

private:
	// 文字列からマップチップの種類へ変換
	MapChipType ParseMapChipType(const std::string& word) const;

private:
	// マップチップデータ
	std::vector<std::vector<MapChipType>> mapChipData_;

	// 1ブロックの幅と高さ
	static inline const float kBlockWidth = 1.0f;
	static inline const float kBlockHeight = 1.0f;

};