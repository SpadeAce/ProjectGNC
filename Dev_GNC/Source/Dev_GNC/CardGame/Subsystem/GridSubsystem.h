// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CardGameTypes.h"
#include "EdgeTileKey.h"
#include "GridSubsystem.generated.h"

class ASquareTile;
class AEdgeTile;
class AGridEntity;
class UTileMapPreset;
class UEntityCatalog;
struct FTilePresetData;

/**
 * UGridSubsystem
 * 타일맵 생성·관리·경로탐색을 담당하는 월드 서브시스템.
 * Unity TileManager (MonoSingleton) 대응.
 * 레벨 전환 시 자동 생성/파괴되어 IResettable.ResetAll() 불필요.
 */
UCLASS()
class UGridSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ════════════════════════════════════════════
	// 방향 상수
	// ════════════════════════════════════════════
	static const TArray<FIntPoint>& GetSquareDirections();
	static FIntPoint DirectionToOffset(ETileDirection Dir);
	static ETileDirection OffsetToDirection(FIntPoint Offset);

	// ════════════════════════════════════════════
	// 프로퍼티
	// ════════════════════════════════════════════
	UFUNCTION(BlueprintPure, Category = "CardGame")
	float GetTileSize() const { return TileSize; }

	UFUNCTION(BlueprintPure, Category = "CardGame")
	int32 GetTileCount() const { return TilesByPosition.Num(); }

	// ════════════════════════════════════════════
	// 맵 생성
	// ════════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	bool GenerateTileMap(int32 Width, int32 Height, float InTileSize = 1.0f);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	bool GenerateTileMapFromPreset(UTileMapPreset* Preset);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void ClearAllTiles();

	// ════════════════════════════════════════════
	// 타일 조회
	// ════════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	ASquareTile* GetTile(FIntPoint GridPosition) const;

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	ASquareTile* GetTileById(int32 Id) const;

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	TArray<ASquareTile*> GetAdjacentTiles(FIntPoint GridPosition) const;

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	TArray<ASquareTile*> GetTilesInRange(FIntPoint Center, int32 Range) const;

	// ════════════════════════════════════════════
	// EdgeTile 조회
	// ════════════════════════════════════════════
	AEdgeTile* GetEdgeTile(FIntPoint GridPos, ETileDirection Direction) const;
	TArray<AEdgeTile*> GetEdgeTilesForTile(FIntPoint GridPosition) const;

	// ════════════════════════════════════════════
	// 경로 탐색
	// ════════════════════════════════════════════
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	TArray<ASquareTile*> GetReachableTiles(FIntPoint Center, int32 Movement, UClass* FriendlyDataClass = nullptr);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	TArray<ASquareTile*> GetPath(FIntPoint From, FIntPoint To, int32 Movement, UClass* FriendlyDataClass = nullptr);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	TArray<ASquareTile*> SmoothPath(const TArray<ASquareTile*>& Path, FIntPoint FromPos, UClass* FriendlyDataClass = nullptr);

	// ════════════════════════════════════════════
	// 유틸리티
	// ════════════════════════════════════════════
	UFUNCTION(BlueprintPure, Category = "CardGame")
	int32 GetDistance(FIntPoint A, FIntPoint B) const;

	UFUNCTION(BlueprintPure, Category = "CardGame")
	FVector GridToWorld(FIntPoint GridPosition) const;

	UFUNCTION(BlueprintPure, Category = "CardGame")
	FIntPoint WorldToGrid(FVector WorldPosition) const;

	UFUNCTION(BlueprintPure, Category = "CardGame")
	FVector GetMapCenter() const;

	/** 맵의 월드 좌표 바운드를 반환한다. */
	UFUNCTION(BlueprintPure, Category = "CardGame")
	void GetMapBounds(FVector2D& OutMin, FVector2D& OutMax) const;

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void DeselectAllTiles();

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	TArray<FTilePresetData> GetSpawnPoints(UTileMapPreset* Preset, ESpawnPointType Type) const;

private:
	float TileSize = 1.0f;

	TMap<FIntPoint, TObjectPtr<ASquareTile>> TilesByPosition;
	TMap<int32, TObjectPtr<ASquareTile>> TilesById;
	TMap<FEdgeTileKey, TObjectPtr<AEdgeTile>> EdgeTiles;
	int32 NextTileId = 0;

	UPROPERTY()
	TObjectPtr<UEntityCatalog> CachedEntityCatalog;

	UPROPERTY()
	TSubclassOf<ASquareTile> SquareTileClass;

	// ── 내부 메서드 ──
	ASquareTile* CreateTile(FIntPoint GridPosition, ETileType Type = ETileType::Ground);
	void GenerateEdgeTiles();
	AEdgeTile* CreateEdgeTile(const FEdgeTileKey& Key, FIntPoint FromPos, FIntPoint DirOffset);
	bool IsEdgePassable(FIntPoint A, FIntPoint B) const;
	bool HasLineOfSight(FIntPoint From, FIntPoint To, UClass* FriendlyDataClass) const;
	AActor* SpawnEntityFromCatalog(int32 EntityId);
};
