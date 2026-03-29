// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DObject.h"
#include "CardGameTypes.h"
#include "BuffEntry.h"
#include "FPawnStats.h"
#include "DPawn.generated.h"

struct FPawnDataRow;
class UDCard;
class UDEquipment;
class UDataSubsystem;
class UTextSubsystem;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStatsChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFloatingText, EFloatingTextType, TextType, int32, Value);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPawnHandChanged);

/**
 * UDPawn
 * 플레이어 유닛 데이터. 스탯, 장비, 카드핸드, 버프, 레벨링을 관리한다.
 */
UCLASS(BlueprintType)
class UDPawn : public UDObject
{
	GENERATED_BODY()

public:
	void InitFromId(int32 InPawnId, UDataSubsystem* DataSub, UTextSubsystem* TextSub);
	void InitFromData(const FPawnDataRow* InData, UDataSubsystem* DataSub, UTextSubsystem* TextSub);

	// ── 식별 ────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 PawnId = 0;
	const FPawnDataRow* Data = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") FString CodeName;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") FString IconPath;

	// ── 런타임 전투 스탯 ────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 HP = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Shield = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Armor = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Movement = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Attack = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Range = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Sight = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Ammo = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Accuracy = 0;

	// ── 레벨링 ──────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Level = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Exp = 0;
	void AddExp(int32 Amount);

	// ── 버프 ────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") TArray<FBuffEntry> Buffs;
	void ApplyBuff(ECardEffectType Type, int32 Value, int32 Duration);
	void TickBuffs();

	// ── 장비 ────────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") TArray<TObjectPtr<UDEquipment>> Equips;

	/** 장비 장착. 슬롯 타입 검증은 호출부에서 수행. */
	bool Equip(UDEquipment* Equipment);

	/** 장비 해제. */
	bool Unequip(UDEquipment* Equipment);

	// ── 계산된 스탯 ─────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") FPawnStats Stats;

	// ── 카드 핸드 ───────────────────────────────────
	static constexpr int32 InitialHandSize = 2;
	static constexpr int32 DrawPerTurn = 1;
	static constexpr int32 BaseMaxHandSize = 3;

	int32 GetMaxHandSize() const { return BaseMaxHandSize + Stats.CardCap; }

	UPROPERTY(BlueprintReadOnly, Category = "CardGame") TArray<TObjectPtr<UDCard>> PawnHandCards;
	void AddPawnHandCard(UDCard* Card);
	void RemovePawnHandCard(UDCard* Card);
	void BuildCardPool();
	void InitHand();
	void DrawCards(int32 Count);
	void UsePawnCard(UDCard* Card);

	// ── 전투 ────────────────────────────────────────
	void TakeDamage(int32 AttackPower);
	void Heal(int32 Amount);
	void AddShield(int32 Amount);
	bool HasEnoughAmmo(int32 Cost) const { return Ammo >= Cost; }
	void ConsumeAmmo(int32 Amount);
	void RestoreAmmo();
	void AddAmmo(int32 Amount);
	bool IsDead() const { return HP <= 0; }
	void ResetStats();

	// ── 델리게이트 ──────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "CardGame") FOnStatsChanged OnStatsChanged;
	UPROPERTY(BlueprintAssignable, Category = "CardGame") FOnFloatingText OnFloatingText;
	UPROPERTY(BlueprintAssignable, Category = "CardGame") FOnPawnHandChanged OnPawnHandChanged;

private:
	TArray<TObjectPtr<UDCard>> PawnCardPool;
	TArray<TObjectPtr<UDCard>> PawnDiscardPool;
	TArray<TPair<EStatusType, int32>> GrowthBonuses;
	TArray<int32> GrowthCardIds;

	TWeakObjectPtr<UDataSubsystem> DataSubRef;

	void UpdateStatus();
	void RecalculateStats();
	void ApplyStatDelta(ECardEffectType Type, int32 Delta);
	void TryLevelUp();
	void ShufflePool(TArray<TObjectPtr<UDCard>>& Pool);
	void RefillFromDiscard();
};
