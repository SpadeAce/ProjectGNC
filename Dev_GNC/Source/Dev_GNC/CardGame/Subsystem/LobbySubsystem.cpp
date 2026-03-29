// Copyright GNC Project. All Rights Reserved.

#include "LobbySubsystem.h"
#include "DataSubsystem.h"
#include "TextSubsystem.h"
#include "DPawn.h"
#include "DCard.h"
#include "CardGameRowTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogLobbySubsystem, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void ULobbySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogLobbySubsystem, Log, TEXT("ULobbySubsystem Initialized"));
}

void ULobbySubsystem::Deinitialize()
{
	ShopCards.Empty();
	RecruitPawns.Empty();
	Super::Deinitialize();
}

// ────────────────────────────────────────────────────────────
// A. 상점 시스템
// ────────────────────────────────────────────────────────────

void ULobbySubsystem::GenerateShopList()
{
	CacheSubsystems();
	ShopCards.Empty();

	UDataSubsystem* DataSub = DataSubRef.Get();
	if (!DataSub)
	{
		UE_LOG(LogLobbySubsystem, Warning, TEXT("GenerateShopList: DataSubsystem is null"));
		return;
	}

	const FShopDataRow* ShopData = DataSub->GetShopDataByLevel(ShopLevel);
	if (!ShopData)
	{
		UE_LOG(LogLobbySubsystem, Warning, TEXT("GenerateShopList: No ShopData for Level %d"), ShopLevel);
		return;
	}

	const int32 Count = FMath::Min(ShopData->CardId.Num(), ShopMaxItemCount);
	for (int32 i = 0; i < Count; ++i)
	{
		const FCardDataRow* CardData = DataSub->GetCardData(ShopData->CardId[i]);
		if (!CardData) continue;

		UDCard* Card = NewObject<UDCard>(this);
		Card->InitFromData(CardData);
		ShopCards.Add(Card);
	}

	UE_LOG(LogLobbySubsystem, Log, TEXT("GenerateShopList: Level=%d, Cards=%d"), ShopLevel, ShopCards.Num());
	OnShopListChanged.Broadcast();
}

bool ULobbySubsystem::LevelUpShop()
{
	CacheSubsystems();
	UDataSubsystem* DataSub = DataSubRef.Get();
	if (!DataSub) return false;

	const int32 MaxLevel = DataSub->GetShopMaxLevel();
	if (ShopLevel >= MaxLevel) return false;

	ShopLevel++;
	GenerateShopList();

	UE_LOG(LogLobbySubsystem, Log, TEXT("LevelUpShop → Level %d"), ShopLevel);
	return true;
}

void ULobbySubsystem::RemoveShopCard(UDCard* Card)
{
	if (!Card) return;
	ShopCards.Remove(Card);
	OnShopListChanged.Broadcast();
}

// ────────────────────────────────────────────────────────────
// B. 모집 시스템
// ────────────────────────────────────────────────────────────

void ULobbySubsystem::GenerateRecruitList()
{
	CacheSubsystems();
	RecruitPawns.Empty();

	UDataSubsystem* DataSub = DataSubRef.Get();
	UTextSubsystem* TextSub = TextSubRef.Get();
	if (!DataSub)
	{
		UE_LOG(LogLobbySubsystem, Warning, TEXT("GenerateRecruitList: DataSubsystem is null"));
		return;
	}

	const TMap<int32, const FPawnDataRow*>& AllPawns = DataSub->GetAllPawnData();
	if (AllPawns.Num() == 0) return;

	// TMap → TArray 변환 (값만)
	TArray<const FPawnDataRow*> PawnArray;
	AllPawns.GenerateValueArray(PawnArray);

	for (int32 i = 0; i < RecruitSlotCount; ++i)
	{
		const int32 Idx = FMath::RandRange(0, PawnArray.Num() - 1);
		UDPawn* Pawn = NewObject<UDPawn>(this);
		Pawn->InitFromData(PawnArray[Idx], DataSub, TextSub);
		RecruitPawns.Add(Pawn);
	}

	UE_LOG(LogLobbySubsystem, Log, TEXT("GenerateRecruitList: Pawns=%d"), RecruitPawns.Num());
	OnRecruitListChanged.Broadcast();
}

void ULobbySubsystem::RemoveRecruitPawn(UDPawn* Pawn)
{
	if (!Pawn) return;
	RecruitPawns.Remove(Pawn);
	OnRecruitListChanged.Broadcast();
}

// ────────────────────────────────────────────────────────────
// C. 리셋
// ────────────────────────────────────────────────────────────

void ULobbySubsystem::ResetForNextStage()
{
	ShopCards.Empty();
	RecruitPawns.Empty();
	UE_LOG(LogLobbySubsystem, Log, TEXT("ResetForNextStage"));
}

void ULobbySubsystem::ResetAll()
{
	ShopLevel = 1;
	ShopCards.Empty();
	RecruitPawns.Empty();
	UE_LOG(LogLobbySubsystem, Log, TEXT("ResetAll"));
}

// ────────────────────────────────────────────────────────────
// D. 로비 진입
// ────────────────────────────────────────────────────────────

void ULobbySubsystem::EnsureLobbyData()
{
	CacheSubsystems();

	if (ShopCards.Num() == 0)
	{
		GenerateShopList();
	}
	if (RecruitPawns.Num() == 0)
	{
		GenerateRecruitList();
	}
}

// ────────────────────────────────────────────────────────────
// Private
// ────────────────────────────────────────────────────────────

void ULobbySubsystem::CacheSubsystems()
{
	if (!DataSubRef.IsValid())
	{
		DataSubRef = GetGameInstance()->GetSubsystem<UDataSubsystem>();
	}
	if (!TextSubRef.IsValid())
	{
		(void)DataSubRef.IsValid(); // 순서 보장
		TextSubRef = GetGameInstance()->GetSubsystem<UTextSubsystem>();
	}
}
