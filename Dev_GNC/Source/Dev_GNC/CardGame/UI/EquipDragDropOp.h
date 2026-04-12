// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "EquipDragDropOp.generated.h"

class UDEquipment;

/**
 * UEquipDragDropOp
 * 장비 드래그 시 payload를 운반하는 드래그 오퍼레이션.
 * Unity PawnManageEquipItem 드래그 이벤트 대응.
 */
UCLASS()
class UEquipDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 드래그 중인 장비 */
	UPROPERTY(BlueprintReadWrite, Category = "CardGame|UI")
	TObjectPtr<UDEquipment> Equipment;
};
