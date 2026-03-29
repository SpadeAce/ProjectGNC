// Copyright GNC Project. All Rights Reserved.

#include "GridEntity.h"
#include "GridSlotComponent.h"

AGridEntity::AGridEntity()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
}

void AGridEntity::SetSlot(UGridSlotComponent* Slot)
{
	CurrentSlot = Slot;

	if (Slot && Slot->GetOwner())
	{
		SetActorLocation(Slot->GetOwner()->GetActorLocation());
		SetActorRotation(FRotator::ZeroRotator);
	}
}

void AGridEntity::SetData(UDObject* InData)
{
	Data = InData;
}
