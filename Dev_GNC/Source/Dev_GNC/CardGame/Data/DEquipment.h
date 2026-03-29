// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "DObject.h"
#include "DEquipment.generated.h"

struct FEquipmentDataRow;
class UDataSubsystem;

/**
 * UDEquipment
 * 장비 인스턴스. 장착된 폰의 InstanceId를 추적한다.
 */
UCLASS(BlueprintType)
class UDEquipment : public UDObject
{
	GENERATED_BODY()

public:
	void InitFromId(int32 InEquipId, UDataSubsystem* DataSub);
	void InitFromData(const FEquipmentDataRow* InData);

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	int32 EquipmentId = 0;

	const FEquipmentDataRow* Data = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	int64 EquippedPawnId = 0;

	bool IsEquipped() const { return EquippedPawnId != 0; }
	void SetEquippedPawn(int64 PawnInstanceId) { EquippedPawnId = PawnInstanceId; }
	void ClearEquippedPawn() { EquippedPawnId = 0; }
};
