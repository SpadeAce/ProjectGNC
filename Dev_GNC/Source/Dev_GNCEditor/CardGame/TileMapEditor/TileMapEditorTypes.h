// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CardGameTypes.h"
#include "EdgeTileKey.h"

// 편집 모드
enum class ETileMapEditMode : uint8
{
	TileType,
	SpawnPoint,
	Entity
};

// 히트 테스트 결과 타입
enum class ETileMapHitType : uint8
{
	None,
	Tile,
	EastEdge,
	NorthEdge
};

// 히트 테스트 결과
struct FTileMapHitResult
{
	ETileMapHitType HitType = ETileMapHitType::None;
	FIntPoint TileCoord = FIntPoint::ZeroValue;
	FEdgeTileKey EdgeKey;
};

// 그리드 렌더링 상수
namespace TileMapEditorConstants
{
	constexpr float CellSize = 32.0f;
	constexpr float EdgeSize = 12.0f;
	constexpr float CellStride = CellSize + EdgeSize; // 44.0f

	inline FLinearColor GetTileColor(ETileType Type)
	{
		switch (Type)
		{
		case ETileType::Empty:   return FLinearColor(0.5f, 0.5f, 0.5f);
		case ETileType::Ground:  return FLinearColor(0.3f, 0.7f, 0.3f);
		case ETileType::Dirt:    return FLinearColor(0.6f, 0.4f, 0.2f);
		case ETileType::Water:   return FLinearColor(0.2f, 0.4f, 0.8f);
		case ETileType::Blocked: return FLinearColor(0.8f, 0.2f, 0.2f);
		default:                 return FLinearColor(0.5f, 0.5f, 0.5f);
		}
	}

	const FLinearColor EdgeEmptyColor(0.15f, 0.15f, 0.15f);
	const FLinearColor EdgeOccupiedColor(1.0f, 0.6f, 0.0f);
	const FLinearColor BorderColor(0.0f, 0.0f, 0.0f, 0.3f);
	const FLinearColor SpawnPlayerColor(0.0f, 1.0f, 1.0f);    // Cyan
	const FLinearColor SpawnEnemyColor(1.0f, 0.0f, 1.0f);     // Magenta
}
