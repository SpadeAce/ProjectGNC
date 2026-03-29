// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DObject.h"
#include "DCard.generated.h"

struct FCardDataRow;
class UDataSubsystem;

/**
 * UDCard
 * CardData 테이블 행을 참조하는 카드 인스턴스.
 */
UCLASS(BlueprintType)
class UDCard : public UDItem
{
	GENERATED_BODY()

public:
	void InitFromId(int32 InCardId, UDataSubsystem* DataSub);
	void InitFromData(const FCardDataRow* InData);

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	int32 CardId = 0;

	const FCardDataRow* Data = nullptr;
};
