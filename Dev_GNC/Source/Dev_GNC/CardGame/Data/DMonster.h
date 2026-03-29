// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DObject.h"
#include "CardGameTypes.h"
#include "BuffEntry.h"
#include "DPawn.h"
#include "DMonster.generated.h"

struct FMonsterDataRow;
class UDataSubsystem;

/**
 * UDMonster
 * 적 유닛 데이터. 스탯과 버프를 관리한다.
 */
UCLASS(BlueprintType)
class UDMonster : public UDObject
{
	GENERATED_BODY()

public:
	void InitFromId(int32 InMonsterId, UDataSubsystem* DataSub);
	void InitFromData(const FMonsterDataRow* InData);

	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 MonsterId = 0;
	const FMonsterDataRow* Data = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 HP = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Shield = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Armor = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Movement = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Attack = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Range = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Sight = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame") TArray<FBuffEntry> Buffs;

	void TakeDamage(int32 AttackPower);
	void Heal(int32 Amount);
	void AddShield(int32 Amount);
	void ApplyBuff(ECardEffectType Type, int32 Value, int32 Duration);
	void TickBuffs();
	void NotifyMiss();
	bool IsDead() const { return HP <= 0; }

	UPROPERTY(BlueprintAssignable, Category = "CardGame") FOnStatsChanged OnStatsChanged;
	UPROPERTY(BlueprintAssignable, Category = "CardGame") FOnFloatingText OnFloatingText;

private:
	void UpdateStatus();
	void ApplyStatDelta(ECardEffectType Type, int32 Delta);
};
