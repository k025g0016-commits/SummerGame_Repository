#include "MapChipField.h"
#include <cassert>
#include <cmath>
#include <fstream>
#include <sstream>

using namespace KamataEngine;

void MapChipField::LoadMapChipCsv(const std::string& filePath)
{
	std::ifstream file(filePath);

	assert(file.is_open());

	mapChipData_.clear();

	std::string line;

	while (std::getline(file, line))
	{
		std::vector<MapChipType> mapChipTypes;
		std::stringstream lineStream(line);
		std::string word;

		while (std::getline(lineStream, word, ','))
		{
			// Windows形式の改行コードに含まれる\rを削除
			if (!word.empty() && word.back() == '\r') 
			{
				word.pop_back();
			}

			mapChipTypes.push_back(ParseMapChipType(word));
		}

		if (!mapChipTypes.empty())
		{
			mapChipData_.push_back(mapChipTypes);
		}
	}
}

MapChipType MapChipField::GetMapChipTypeByIndex(int32_t xIndex, int32_t yIndex) const
{
	// マップ外の負の番号は空白扱い
	if (xIndex < 0 || yIndex < 0) 
	{
		return MapChipType::kBlank;
	}

	const uint32_t x = static_cast<uint32_t>(xIndex);

	const uint32_t y = static_cast<uint32_t>(yIndex);

	if (y >= mapChipData_.size()) 
	{
		return MapChipType::kBlank;
	}

	if (x >= mapChipData_[y].size())
	{
		return MapChipType::kBlank;
	}

	return mapChipData_[y][x];
}

Vector3 MapChipField::GetMapChipPositionByIndex(uint32_t xIndex, uint32_t yIndex) const
{
	// CSVの上側をゲーム画面でも上側として扱うため、Y座標を反転する
	const float x = static_cast<float>(xIndex) * kBlockWidth;

	const float y = static_cast<float>(GetNumBlockVertical() - 1 - yIndex) * kBlockHeight;

	return {x, y, 0.0f};
}

MapChipRect MapChipField::GetRectByIndex(int32_t xIndex, int32_t yIndex) const
{
	MapChipRect rect{};

	// マップ外なら空の矩形を返す
	if (xIndex < 0 || yIndex < 0 || xIndex >= static_cast<int32_t>(GetNumBlockHorizontal()) || yIndex >= static_cast<int32_t>(GetNumBlockVertical())) 
	{
		return rect;
	}

	const Vector3 position = GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex));

	rect.left = position.x - kBlockWidth / 2.0f;

	rect.right = position.x + kBlockWidth / 2.0f;

	rect.bottom = position.y - kBlockHeight / 2.0f;

	rect.top = position.y + kBlockHeight / 2.0f;

	return rect;
}

MapChipIndexSet MapChipField::GetMapChipIndexSetByPosition(const Vector3& position) const 
{
	MapChipIndexSet indexSet{};

	// X方向は左から右へ番号が増える
	indexSet.xIndex = static_cast<int32_t>(std::floor((position.x + kBlockWidth / 2.0f) / kBlockWidth));

	// ワールド座標上で下から何番目かを計算
	const int32_t yIndexFromBottom = static_cast<int32_t>(std::floor((position.y + kBlockHeight / 2.0f) / kBlockHeight));

	// CSVは上から下へ番号が増えるため反転する
	indexSet.yIndex = static_cast<int32_t>(GetNumBlockVertical()) - 1 - yIndexFromBottom;

	return indexSet;
}

Vector3 MapChipField::GetPlayerPosition() const
{
	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kPlayer) 
			{
				return GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex));
			}
		}
	}

	// P0が見つからなかった場合
	return {0.0f, 1.0f, 0.0f};
}

uint32_t MapChipField::GetNumBlockHorizontal() const
{
	uint32_t maxSize = 0;

	for (const std::vector<MapChipType>& row : mapChipData_)
	{
		const uint32_t rowSize = static_cast<uint32_t>(row.size());

		if (rowSize > maxSize) 
		{
			maxSize = rowSize;
		}
	}

	return maxSize;
}

uint32_t MapChipField::GetNumBlockVertical() const { return static_cast<uint32_t>(mapChipData_.size()); }

MapChipType MapChipField::ParseMapChipType(const std::string& word) const 
{
	if (word == "B0")
	{
		return MapChipType::kBlock;
	}

	if (word == "P0")
	{
		return MapChipType::kPlayer;
	}

	return MapChipType::kBlank;
}