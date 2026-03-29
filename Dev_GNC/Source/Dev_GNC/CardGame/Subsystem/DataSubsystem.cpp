// Copyright GNC Project. All Rights Reserved.

#include "DataSubsystem.h"

// ---------------------------------------------------------------------------
// 헬퍼: DataTable 로드 + ID 캐시
// ---------------------------------------------------------------------------

template<typename TRow>
UDataTable* UDataSubsystem::LoadTable(const TCHAR* AssetPath, TMap<int32, const TRow*>& OutCache)
{
	UDataTable* Table = LoadObject<UDataTable>(nullptr, AssetPath);
	if (!Table)
	{
		UE_LOG(LogTemp, Warning, TEXT("[DataSubsystem] DataTable 로드 실패: %s"), AssetPath);
		return nullptr;
	}

	const TMap<FName, uint8*>& RowMap = Table->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const TRow* Row = reinterpret_cast<const TRow*>(Pair.Value);
		if (Row)
		{
			OutCache.Add(Row->Id, Row);
		}
	}

	return Table;
}

// ---------------------------------------------------------------------------
// Initialize / Deinitialize
// ---------------------------------------------------------------------------

void UDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	PawnDataTable = LoadTable<FPawnDataRow>(
		TEXT("/Game/CardGame/DataTables/DT_PawnData.DT_PawnData"), PawnCache);
	MonsterDataTable = LoadTable<FMonsterDataRow>(
		TEXT("/Game/CardGame/DataTables/DT_MonsterData.DT_MonsterData"), MonsterCache);
	CardDataTable = LoadTable<FCardDataRow>(
		TEXT("/Game/CardGame/DataTables/DT_CardData.DT_CardData"), CardCache);
	CardEffectDataTable = LoadTable<FCardEffectRow>(
		TEXT("/Game/CardGame/DataTables/DT_CardEffectData.DT_CardEffectData"), CardEffectCache);
	EquipmentDataTable = LoadTable<FEquipmentDataRow>(
		TEXT("/Game/CardGame/DataTables/DT_EquipmentData.DT_EquipmentData"), EquipmentCache);
	TileEntityDataTable = LoadTable<FTileEntityRow>(
		TEXT("/Game/CardGame/DataTables/DT_TileEntityData.DT_TileEntityData"), TileEntityCache);
	StagePresetDataTable = LoadTable<FStagePresetRow>(
		TEXT("/Game/CardGame/DataTables/DT_StagePresetData.DT_StagePresetData"), StagePresetCache);
	StageRewardDataTable = LoadTable<FStageRewardRow>(
		TEXT("/Game/CardGame/DataTables/DT_StageRewardData.DT_StageRewardData"), StageRewardCache);
	ShopDataTable = LoadTable<FShopDataRow>(
		TEXT("/Game/CardGame/DataTables/DT_ShopData.DT_ShopData"), ShopCache);
	PawnGrowthDataTable = LoadTable<FPawnGrowthRow>(
		TEXT("/Game/CardGame/DataTables/DT_PawnGrowthData.DT_PawnGrowthData"), PawnGrowthCache);
	NamePresetDataTable = LoadTable<FNamePresetRow>(
		TEXT("/Game/CardGame/DataTables/DT_NamePresetData.DT_NamePresetData"), NamePresetCache);

	BuildSecondaryIndices();

	UE_LOG(LogTemp, Log, TEXT("[DataSubsystem] 초기화 완료 — Pawn:%d, Monster:%d, Card:%d, Equipment:%d"),
		PawnCache.Num(), MonsterCache.Num(), CardCache.Num(), EquipmentCache.Num());
}

void UDataSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// 2차 인덱스
// ---------------------------------------------------------------------------

void UDataSubsystem::BuildSecondaryIndices()
{
	// Shop: Level 기반
	ShopMaxLevel = 0;
	for (const auto& Pair : ShopCache)
	{
		ShopLevelCache.Add(Pair.Value->Level, Pair.Value);
		ShopMaxLevel = FMath::Max(ShopMaxLevel, Pair.Value->Level);
	}

	// StagePreset: Level 기반
	for (const auto& Pair : StagePresetCache)
	{
		StagePresetLevelCache.Add(Pair.Value->Level, Pair.Value);
	}

	// StageReward: Level 기반
	for (const auto& Pair : StageRewardCache)
	{
		StageRewardLevelCache.Add(Pair.Value->Level, Pair.Value);
	}

	// PawnGrowth: ClassType * 1000 + Level 복합키
	for (const auto& Pair : PawnGrowthCache)
	{
		const int32 Key = static_cast<int32>(Pair.Value->ClassType) * 1000 + Pair.Value->Level;
		PawnGrowthClassLevelCache.Add(Key, Pair.Value);
	}

	// NamePreset: 배열화 (랜덤 접근용)
	NamePresetCache.GenerateValueArray(NamePresetList);
}

// ---------------------------------------------------------------------------
// ID 기반 조회
// ---------------------------------------------------------------------------

const FPawnDataRow* UDataSubsystem::GetPawnData(int32 Id) const
{
	const auto* Found = PawnCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FMonsterDataRow* UDataSubsystem::GetMonsterData(int32 Id) const
{
	const auto* Found = MonsterCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FCardDataRow* UDataSubsystem::GetCardData(int32 Id) const
{
	const auto* Found = CardCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FCardEffectRow* UDataSubsystem::GetCardEffectData(int32 Id) const
{
	const auto* Found = CardEffectCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FEquipmentDataRow* UDataSubsystem::GetEquipmentData(int32 Id) const
{
	const auto* Found = EquipmentCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FTileEntityRow* UDataSubsystem::GetTileEntityData(int32 Id) const
{
	const auto* Found = TileEntityCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FStagePresetRow* UDataSubsystem::GetStagePresetData(int32 Id) const
{
	const auto* Found = StagePresetCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FStageRewardRow* UDataSubsystem::GetStageRewardData(int32 Id) const
{
	const auto* Found = StageRewardCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FShopDataRow* UDataSubsystem::GetShopData(int32 Id) const
{
	const auto* Found = ShopCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FPawnGrowthRow* UDataSubsystem::GetPawnGrowthData(int32 Id) const
{
	const auto* Found = PawnGrowthCache.Find(Id);
	return Found ? *Found : nullptr;
}

const FNamePresetRow* UDataSubsystem::GetNamePresetData(int32 Id) const
{
	const auto* Found = NamePresetCache.Find(Id);
	return Found ? *Found : nullptr;
}

// ---------------------------------------------------------------------------
// 특수 조회
// ---------------------------------------------------------------------------

const FShopDataRow* UDataSubsystem::GetShopDataByLevel(int32 Level) const
{
	const auto* Found = ShopLevelCache.Find(Level);
	return Found ? *Found : nullptr;
}

int32 UDataSubsystem::GetShopMaxLevel() const
{
	return ShopMaxLevel;
}

const FStagePresetRow* UDataSubsystem::GetStagePresetByLevel(int32 Level) const
{
	const auto* Found = StagePresetLevelCache.Find(Level);
	return Found ? *Found : nullptr;
}

const FStageRewardRow* UDataSubsystem::GetStageRewardByLevel(int32 Level) const
{
	const auto* Found = StageRewardLevelCache.Find(Level);
	return Found ? *Found : nullptr;
}

const FPawnGrowthRow* UDataSubsystem::GetPawnGrowthByClassLevel(EClassType ClassType, int32 Level) const
{
	const int32 Key = static_cast<int32>(ClassType) * 1000 + Level;
	const auto* Found = PawnGrowthClassLevelCache.Find(Key);
	return Found ? *Found : nullptr;
}

FString UDataSubsystem::GetRandomName() const
{
	if (NamePresetList.Num() == 0)
	{
		return TEXT("Unknown");
	}
	const int32 Index = FMath::RandRange(0, NamePresetList.Num() - 1);
	return NamePresetList[Index]->Name;
}
