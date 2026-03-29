// Copyright GNC Project. All Rights Reserved.

#include "EdgeTile.h"
#include "GridSlotComponent.h"

AEdgeTile::AEdgeTile()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Slot = CreateDefaultSubobject<UGridSlotComponent>(TEXT("Slot"));
}

void AEdgeTile::Init(const FEdgeTileKey& InKey, ETileDirection InDirection)
{
	Key = InKey;
	Direction = InDirection;
}

void AEdgeTile::Clear()
{
	if (Slot)
	{
		Slot->ClearEntity();
	}
}
