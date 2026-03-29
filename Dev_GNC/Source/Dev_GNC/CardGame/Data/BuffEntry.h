// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CardGameTypes.h"
#include "BuffEntry.generated.h"

USTRUCT(BlueprintType)
struct FBuffEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	ECardEffectType Type = ECardEffectType::None;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	int32 Value = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	int32 RemainingTurns = 0;

	FBuffEntry() = default;

	FBuffEntry(ECardEffectType InType, int32 InValue, int32 InTurns)
		: Type(InType), Value(InValue), RemainingTurns(InTurns)
	{
	}
};
