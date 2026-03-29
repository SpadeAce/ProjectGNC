// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CardGameTypes.h"
#include "TileMapPreset.generated.h"

USTRUCT(BlueprintType)
struct FTilePresetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FIntPoint Position = FIntPoint::ZeroValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ETileType TileType = ETileType::Ground;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ESpawnPointType SpawnPoint = ESpawnPointType::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 SpawnId = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 EntityId = 0;
};

USTRUCT(BlueprintType)
struct FEdgePresetData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FIntPoint PosA = FIntPoint::ZeroValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FIntPoint PosB = FIntPoint::ZeroValue;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 EntityId = 0;
};

/**
 * UTileMapPreset
 * 타일맵 프리셋 데이터 에셋. Unity TileMapPreset (ScriptableObject) 대응.
 */
UCLASS(BlueprintType)
class UTileMapPreset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame") int32 Width = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame") int32 Height = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame") float TileSize = 105.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame") int32 TurnLimit = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame") int32 MaxPawnCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame")
	TArray<FTilePresetData> Tiles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame")
	TArray<FEdgePresetData> Edges;

	const FTilePresetData* GetTileData(FIntPoint Position) const;
};
