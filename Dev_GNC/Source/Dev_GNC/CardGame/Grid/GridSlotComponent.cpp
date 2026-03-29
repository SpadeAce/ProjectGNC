// Copyright GNC Project. All Rights Reserved.

#include "GridSlotComponent.h"
#include "GridEntity.h"

UGridSlotComponent::UGridSlotComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UGridSlotComponent::SetEntity(AGridEntity* Entity)
{
	if (Entity == nullptr) return;

	SlotEntity = Entity;
	Entity->SetSlot(this);
}

void UGridSlotComponent::ClearEntity()
{
	SlotEntity = nullptr;
}
