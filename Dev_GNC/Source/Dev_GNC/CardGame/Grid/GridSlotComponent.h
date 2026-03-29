// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GridSlotComponent.generated.h"

class AGridEntity;

/**
 * UGridSlotComponent
 * 타일/엣지 타일에 부착되어 엔티티(AGridEntity) 배치를 관리하는 슬롯.
 * Unity TileSlot (MonoBehaviour) 대응.
 */
UCLASS(ClassGroup=(CardGame), meta=(BlueprintSpawnableComponent))
class UGridSlotComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGridSlotComponent();

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	TObjectPtr<AGridEntity> SlotEntity;

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetEntity(AGridEntity* Entity);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void ClearEntity();

	UFUNCTION(BlueprintPure, Category = "CardGame")
	bool IsOccupied() const { return SlotEntity != nullptr; }
};
