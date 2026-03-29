// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CardGameTypes.h"
#include "FPawnStats.generated.h"

struct FPawnDataRow;
struct FEquipmentDataRow;

/**
 * FPawnStats
 * 기본 스탯 + 장비 보너스 + 성장 보너스를 합산하여 최종 스탯을 계산한다.
 * Unity PawnStats 클래스 대체.
 */
USTRUCT(BlueprintType)
struct FPawnStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Hp = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Attack = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Armor = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Shield = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Movement = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Range = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Sight = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Ammo = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 CardCap = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 Accuracy = 0;

	/** 기본 데이터 + 장비 + 성장 보너스로부터 최종 스탯을 재계산한다. */
	void Recalculate(
		const FPawnDataRow& BaseData,
		const TArray<const FEquipmentDataRow*>& Equips,
		const TArray<TPair<EStatusType, int32>>& GrowthBonuses);

private:
	void ApplyStatusBonus(EStatusType Type, int32 Value);
};
