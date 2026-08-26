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

	mapChipGroupIds_.clear();

	std::string line;

	while (std::getline(file, line))
	{
		std::vector<MapChipType> mapChipTypes;
		std::vector<int32_t> groupIds;
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
			groupIds.push_back(ParseGroupId(word));
		}

		if (!mapChipTypes.empty()) 
		{
			mapChipData_.push_back(mapChipTypes);
			mapChipGroupIds_.push_back(groupIds);
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

	if (word == "B1") 
	{
		return MapChipType::kSpikeBlock;
	}

	if (word == "P0")
	{
		return MapChipType::kPlayer;
	}

	if (word == "E0")
	{
		return MapChipType::kEnemy;
	}

	if (word == "E1")
	{
		return MapChipType::kArrowEnemy;
	}

	if (word == "E2")
	{
		return MapChipType::kShieldEnemy;
	}

	if (word == "E3")
	{
		return MapChipType::kBoss;
	}

	if (word == "A0") 
	{
		return MapChipType::kBossArea;
	}

	if (word == "G0" || word.starts_with("G0_")) 
	{
		return MapChipType::kSwitch;
	}

	if (word == "G1" || word.starts_with("G1_"))
	{
		return MapChipType::kShutterDoor;
	}

	if (word == "G2")
	{
		return MapChipType::kMoveBlock;
	}

	if (word == "R0")
	{
		return MapChipType::kBesideRail;
	}

	if (word == "R1") 
	{
		return MapChipType::kVerticalRail;
	}

	if (word == "T0")
	{
		return MapChipType::kMoveTutorial;
	}

	if (word == "T1")
	{
		return MapChipType::kJumpTutorial;
	}

	if (word == "T2") 
	{
		return MapChipType::kAttackTutorial;
	}

	if (word == "T3") 
	{
		return MapChipType::kDefenceTutorial;
	}

	if (word == "T4") 
	{
		return MapChipType::kPauseTutorial;
	}

	return MapChipType::kBlank;
}

int32_t MapChipField::ParseGroupId(const std::string& word) const
{
	// 従来のG0 / G1はグループ0として扱う
	if (word == "G0" || word == "G1") 
	{
		return 0;
	}

	// G0_番号 または G1_番号
	if (word.starts_with("G0_") || word.starts_with("G1_"))
	{
		const size_t separatorPosition = word.find('_');

		if (separatorPosition != std::string::npos) 
		{
			const std::string numberString = word.substr(separatorPosition + 1);

			if (!numberString.empty())
			{
				return std::stoi(numberString);
			}
		}
	}

	// グループなし
	return -1;
}

std::vector<Vector3> MapChipField::GetEnemyPositions() const 
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex) 
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kEnemy) 
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetArrowEnemyPositions() const 
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kArrowEnemy) 
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetShieldEnemyPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex) 
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kShieldEnemy)
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetSwitchPositions() const 
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());
	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kSwitch)
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<MapChipGroupData> MapChipField::GetSwitchGroupData() const 
{
	std::vector<MapChipGroupData> dataList;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) != MapChipType::kSwitch) 
			{
				continue;
			}

			MapChipGroupData data{};

			data.position = GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex));

			data.groupId = mapChipGroupIds_[yIndex][xIndex];

			dataList.push_back(data);
		}
	}

	return dataList;
}

std::vector<Vector3> MapChipField::GetShutterDoorPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());
	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kShutterDoor)
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<MapChipGroupData> MapChipField::GetShutterDoorGroupData() const
{
	std::vector<MapChipGroupData> dataList;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) != MapChipType::kShutterDoor)
			{
				continue;
			}

			MapChipGroupData data{};

			data.position = GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex));

			data.groupId = mapChipGroupIds_[yIndex][xIndex];

			dataList.push_back(data);
		}
	}

	return dataList;
}

std::vector<Vector3> MapChipField::GetMoveBlockPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());
	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kMoveBlock)
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetBesideRailPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());
	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex) 
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kBesideRail) 
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetVerticalRailPositions() const 
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kVerticalRail)
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetSpikeBlockPositions() const 
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kSpikeBlock) 
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

Vector3 MapChipField::GetBossPosition() const
{
	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex) 
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kBoss) 
			{
				return GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex));
			}
		}
	}

	// E3がなかった場合
	return {0.0f, 0.0f, 0.0f};
}

std::vector<Vector3> MapChipField::GetBossAreaPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex) 
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kBossArea) 
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

Vector3 MapChipField::GetBossAreaPosition() const
{
	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kBossArea) 
			{
				return GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex));
			}
		}
	}

	return {0.0f, 0.0f, 0.0f};
}

std::vector<Vector3> MapChipField::GetMoveTutorialPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kMoveTutorial)
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetJumpTutorialPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kJumpTutorial) 
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetAttackTutorialPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kAttackTutorial) 
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetDefenceTutorialPositions() const
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex) 
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kDefenceTutorial)
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

std::vector<Vector3> MapChipField::GetPauseTutorialPositions() const 
{
	std::vector<Vector3> positions;

	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex) 
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex)
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kPauseTutorial)
			{
				positions.push_back(GetMapChipPositionByIndex(static_cast<uint32_t>(xIndex), static_cast<uint32_t>(yIndex)));
			}
		}
	}

	return positions;
}

bool MapChipField::HasBoss() const 
{
	const int32_t numVertical = static_cast<int32_t>(GetNumBlockVertical());

	const int32_t numHorizontal = static_cast<int32_t>(GetNumBlockHorizontal());

	for (int32_t yIndex = 0; yIndex < numVertical; ++yIndex)
	{
		for (int32_t xIndex = 0; xIndex < numHorizontal; ++xIndex) 
		{
			if (GetMapChipTypeByIndex(xIndex, yIndex) == MapChipType::kBoss)
			{
				return true;
			}
		}
	}

	return false;
}