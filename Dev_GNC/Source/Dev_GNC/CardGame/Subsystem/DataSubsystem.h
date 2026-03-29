// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CardGameRowTypes.h"
#include "DataSubsystem.generated.h"

/**
 * UDataSubsystem
 * 모든 CardGame 데이터 테이블을 로드하고 ID 기반 조회를 제공한다.
 * Unity DataManager.Instance.Xxx.Get(id) 패턴 대체.
 */
UCLASS()
class UDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── ID 기반 조회 ────────────────────────────────────
	const FPawnDataRow* GetPawnData(int32 Id) const;
	const FMonsterDataRow* GetMonsterData(int32 Id) const;
	const FCardDataRow* GetCardData(int32 Id) const;
	const FCardEffectRow* GetCardEffectData(int32 Id) const;
	const FEquipmentDataRow* GetEquipmentData(int32 Id) const;
	const FTileEntityRow* GetTileEntityData(int32 Id) const;
	const FStagePresetRow* GetStagePresetData(int32 Id) const;
	const FStageRewardRow* GetStageRewardData(int32 Id) const;
	const FShopDataRow* GetShopData(int32 Id) const;
	const FPawnGrowthRow* GetPawnGrowthData(int32 Id) const;
	const FNamePresetRow* GetNamePresetData(int32 Id) const;

	// ── 특수 조회 ───────────────────────────────────────
	const FShopDataRow* GetShopDataByLevel(int32 Level) const;
	int32 GetShopMaxLevel() const;
	const FStagePresetRow* GetStagePresetByLevel(int32 Level) const;
	const FStageRewardRow* GetStageRewardByLevel(int32 Level) const;
	const FPawnGrowthRow* GetPawnGrowthByClassLevel(EClassType ClassType, int32 Level) const;
	FString GetRandomName() const;

	// ── 전체 목록 ───────────────────────────────────────
	const TMap<int32, const FPawnDataRow*>& GetAllPawnData() const { return PawnCache; }
	const TMap<int32, const FMonsterDataRow*>& GetAllMonsterData() const { return MonsterCache; }
	const TMap<int32, const FCardDataRow*>& GetAllCardData() const { return CardCache; }
	const TMap<int32, const FEquipmentDataRow*>& GetAllEquipmentData() const { return EquipmentCache; }

private:
	// DataTable 에셋 포인터
	UPROPERTY() TObjectPtr<UDataTable> PawnDataTable;
	UPROPERTY() TObjectPtr<UDataTable> MonsterDataTable;
	UPROPERTY() TObjectPtr<UDataTable> CardDataTable;
	UPROPERTY() TObjectPtr<UDataTable> CardEffectDataTable;
	UPROPERTY() TObjectPtr<UDataTable> EquipmentDataTable;
	UPROPERTY() TObjectPtr<UDataTable> TileEntityDataTable;
	UPROPERTY() TObjectPtr<UDataTable> StagePresetDataTable;
	UPROPERTY() TObjectPtr<UDataTable> StageRewardDataTable;
	UPROPERTY() TObjectPtr<UDataTable> ShopDataTable;
	UPROPERTY() TObjectPtr<UDataTable> PawnGrowthDataTable;
	UPROPERTY() TObjectPtr<UDataTable> NamePresetDataTable;

	// ID 캐시
	TMap<int32, const FPawnDataRow*> PawnCache;
	TMap<int32, const FMonsterDataRow*> MonsterCache;
	TMap<int32, const FCardDataRow*> CardCache;
	TMap<int32, const FCardEffectRow*> CardEffectCache;
	TMap<int32, const FEquipmentDataRow*> EquipmentCache;
	TMap<int32, const FTileEntityRow*> TileEntityCache;
	TMap<int32, const FStagePresetRow*> StagePresetCache;
	TMap<int32, const FStageRewardRow*> StageRewardCache;
	TMap<int32, const FShopDataRow*> ShopCache;
	TMap<int32, const FPawnGrowthRow*> PawnGrowthCache;
	TMap<int32, const FNamePresetRow*> NamePresetCache;

	// 2차 인덱스
	TMap<int32, const FShopDataRow*> ShopLevelCache;
	int32 ShopMaxLevel = 0;
	TMap<int32, const FStagePresetRow*> StagePresetLevelCache;
	TMap<int32, const FStageRewardRow*> StageRewardLevelCache;
	// Key: ClassType * 1000 + Level
	TMap<int32, const FPawnGrowthRow*> PawnGrowthClassLevelCache;
	TArray<const FNamePresetRow*> NamePresetList;

	// 헬퍼
	template<typename TRow>
	UDataTable* LoadTable(const TCHAR* AssetPath, TMap<int32, const TRow*>& OutCache);

	void BuildSecondaryIndices();
};
