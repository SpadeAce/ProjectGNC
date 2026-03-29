// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EdgeTileKey.h"
#include "CardGameTypes.h"
#include "EdgeTile.generated.h"

class UGridSlotComponent;

/**
 * AEdgeTile
 * 두 인접 타일 사이의 경계(Edge) 액터.
 * 벽, 장애물 등의 AGridEntity를 배치하기 위한 슬롯을 보유한다.
 * Unity EdgeTile (MonoBehaviour) 대응.
 */
UCLASS(BlueprintType)
class AEdgeTile : public AActor
{
	GENERATED_BODY()

public:
	AEdgeTile();

	void Init(const FEdgeTileKey& InKey, ETileDirection InDirection);
	void Clear();

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	FEdgeTileKey Key;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	ETileDirection Direction = ETileDirection::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CardGame")
	TObjectPtr<UGridSlotComponent> Slot;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;
};
