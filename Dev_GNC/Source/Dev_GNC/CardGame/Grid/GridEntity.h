// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridEntity.generated.h"

class UDObject;
class UGridSlotComponent;

/**
 * AGridEntity
 * 타일 위에 배치되는 엔티티의 추상 베이스 클래스.
 * Unity TileEntity (abstract MonoBehaviour) 대응.
 * Phase 3의 ACardGameActor, Phase 2의 ATileWall이 상속한다.
 */
UCLASS(Abstract, BlueprintType)
class AGridEntity : public AActor
{
	GENERATED_BODY()

public:
	AGridEntity();

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	TObjectPtr<UDObject> Data;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	TObjectPtr<UGridSlotComponent> CurrentSlot;

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetSlot(UGridSlotComponent* Slot);

protected:
	void SetData(UDObject* InData);

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;
};
