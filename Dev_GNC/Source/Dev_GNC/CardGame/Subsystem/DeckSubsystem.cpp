// Copyright GNC Project. All Rights Reserved.

#include "DeckSubsystem.h"
#include "DPawn.h"
#include "DCard.h"
#include "DEquipment.h"
#include "CardGameRowTypes.h"
#include "PawnSubsystem.h"
#include "ItemSubsystem.h"
#include "DataSubsystem.h"
#include "TextSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogDeckSubsystem, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void UDeckSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	PawnSlots.SetNum(DeckSlotCount);
	OpenSlotCount = InitialOpenSlots;
	UE_LOG(LogDeckSubsystem, Log, TEXT("UDeckSubsystem Initialized (Slots: %d/%d)"), OpenSlotCount, DeckSlotCount);
}

void UDeckSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ────────────────────────────────────────────────────────────
// A. Pawn 편성 슬롯 관리
// ────────────────────────────────────────────────────────────

bool UDeckSubsystem::ExpandSlot()
{
	if (OpenSlotCount >= DeckSlotCount) return false;
	OpenSlotCount++;
	UE_LOG(LogDeckSubsystem, Log, TEXT("ExpandSlot → %d/%d"), OpenSlotCount, DeckSlotCount);
	return true;
}

bool UDeckSubsystem::AssignPawnToDeck(UDPawn* Pawn, int32 SlotIndex)
{
	if (!Pawn || SlotIndex < 0 || SlotIndex >= OpenSlotCount) return false;
	PawnSlots[SlotIndex] = Pawn;
	UE_LOG(LogDeckSubsystem, Log, TEXT("AssignPawnToDeck: %s → Slot %d"), *Pawn->CodeName, SlotIndex);
	return true;
}

void UDeckSubsystem::UnassignPawnFromDeck(int32 SlotIndex)
{
	if (SlotIndex < 0 || SlotIndex >= DeckSlotCount) return;
	PawnSlots[SlotIndex] = nullptr;
}

int32 UDeckSubsystem::FindEmptySlotIndex() const
{
	for (int32 i = 0; i < OpenSlotCount; ++i)
	{
		if (!PawnSlots[i]) return i;
	}
	return -1;
}

UDPawn* UDeckSubsystem::GetDeckPawn(int32 SlotIndex) const
{
	if (SlotIndex < 0 || SlotIndex >= DeckSlotCount) return nullptr;
	return PawnSlots[SlotIndex];
}

TArray<UDPawn*> UDeckSubsystem::GetActivePawns() const
{
	TArray<UDPawn*> Result;
	for (int32 i = 0; i < OpenSlotCount; ++i)
	{
		if (PawnSlots[i])
		{
			Result.Add(PawnSlots[i]);
		}
	}
	return Result;
}

// ────────────────────────────────────────────────────────────
// B. 글로벌 카드 덱 관리
// ────────────────────────────────────────────────────────────

void UDeckSubsystem::AddCard(UDCard* Card)
{
	if (!Card) return;
	DeckCardList.Add(Card);
	UE_LOG(LogDeckSubsystem, Log, TEXT("AddCard: Id=%d (DeckSize: %d)"), Card->CardId, DeckCardList.Num());
}

void UDeckSubsystem::InitStage()
{
	CardPool.Empty();
	DiscardPool.Empty();
	HandCardList.Empty();

	CardPool.Append(DeckCardList);
	ShufflePool(CardPool);

	UE_LOG(LogDeckSubsystem, Log, TEXT("InitStage: CardPool=%d, Drawing %d"), CardPool.Num(), InitialHandSize);
	DrawCards(InitialHandSize);
}

void UDeckSubsystem::StartTurnDraw()
{
	DrawCards(DrawPerTurn);
}

void UDeckSubsystem::DrawCards(int32 Count)
{
	for (int32 i = 0; i < Count; ++i)
	{
		if (HandCardList.Num() >= MaxHandSize) break;

		if (CardPool.Num() == 0)
		{
			if (DiscardPool.Num() == 0) break;
			RefillFromDiscard();
		}

		UDCard* Card = CardPool.Last();
		CardPool.RemoveAt(CardPool.Num() - 1);
		HandCardList.Add(Card);
	}

	UE_LOG(LogDeckSubsystem, Log, TEXT("DrawCards: Hand=%d, Pool=%d, Discard=%d"),
		HandCardList.Num(), CardPool.Num(), DiscardPool.Num());
	OnHandChanged.Broadcast();
}

void UDeckSubsystem::UseCard(UDCard* Card)
{
	if (!Card || !Card->Data) return;

	HandCardList.Remove(Card);

	if (Card->Data->Type != ECardType::Supply && Card->Data->Type != ECardType::Consume)
	{
		DiscardPool.Add(Card);
	}

	UE_LOG(LogDeckSubsystem, Log, TEXT("UseCard: Id=%d Type=%d → Hand=%d"),
		Card->CardId, static_cast<int32>(Card->Data->Type), HandCardList.Num());
	OnHandChanged.Broadcast();
}

void UDeckSubsystem::InitTestData()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UDataSubsystem* DataSub = GI->GetSubsystem<UDataSubsystem>();
	UTextSubsystem* TextSub = GI->GetSubsystem<UTextSubsystem>();
	UPawnSubsystem* PawnSub = GI->GetSubsystem<UPawnSubsystem>();
	UItemSubsystem* ItemSub = GI->GetSubsystem<UItemSubsystem>();

	// 카드 6장: 10001 x3, 10004 x3
	const TArray<int32> TestCardIds = { 10001, 10001, 10001, 10004, 10004, 10004 };
	for (int32 CardId : TestCardIds)
	{
		UDCard* Card = NewObject<UDCard>(this);
		Card->InitFromId(CardId, DataSub);
		AddCard(Card);
	}

	// 폰 2개 (PawnId=1)
	for (int32 i = 0; i < 2; ++i)
	{
		UDPawn* Pawn = NewObject<UDPawn>(this);
		Pawn->InitFromId(1, DataSub, TextSub);
		if (PawnSub) PawnSub->AddPawn(Pawn);
		AssignPawnToDeck(Pawn, i);
	}

	// 장비 11개
	const TArray<int32> TestEquipIds = {
		10001, 10002, 10003, 10004,
		30001, 30002, 30003,
		50101, 50201, 50301, 60101
	};
	for (int32 EquipId : TestEquipIds)
	{
		UDEquipment* Equip = NewObject<UDEquipment>(this);
		Equip->InitFromId(EquipId, DataSub);
		if (ItemSub) ItemSub->AddEquip(Equip);
	}

	UE_LOG(LogDeckSubsystem, Log, TEXT("InitTestData: Cards=%d, Pawns=%d, Equips=%d"),
		DeckCardList.Num(), PawnSub ? PawnSub->GetPawnCount() : 0, ItemSub ? ItemSub->GetEquipCount() : 0);
}

void UDeckSubsystem::ResetAll()
{
	for (int32 i = 0; i < DeckSlotCount; ++i)
	{
		PawnSlots[i] = nullptr;
	}
	OpenSlotCount = InitialOpenSlots;

	DeckCardList.Empty();
	CardPool.Empty();
	DiscardPool.Empty();
	HandCardList.Empty();

	UE_LOG(LogDeckSubsystem, Log, TEXT("ResetAll"));
}

// ────────────────────────────────────────────────────────────
// Private
// ────────────────────────────────────────────────────────────

void UDeckSubsystem::ShufflePool(TArray<TObjectPtr<UDCard>>& Pool)
{
	const int32 Num = Pool.Num();
	for (int32 i = Num - 1; i > 0; --i)
	{
		const int32 j = FMath::RandRange(0, i);
		Pool.Swap(i, j);
	}
}

void UDeckSubsystem::RefillFromDiscard()
{
	CardPool.Append(DiscardPool);
	DiscardPool.Empty();
	ShufflePool(CardPool);
	UE_LOG(LogDeckSubsystem, Log, TEXT("RefillFromDiscard: CardPool=%d"), CardPool.Num());
}
