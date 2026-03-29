// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridEntity.h"
#include "TileWall.generated.h"

/**
 * ATileWall
 * EdgeTile에 배치되어 이동을 제한하는 벽 엔티티.
 * Unity TileWall : TileEntity 대응.
 */
UCLASS(BlueprintType)
class ATileWall : public AGridEntity
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame")
	bool bIsPassable = false;
};
