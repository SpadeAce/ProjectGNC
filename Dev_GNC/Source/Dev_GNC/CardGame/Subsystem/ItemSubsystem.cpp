// Copyright GNC Project. All Rights Reserved.

#include "ItemSubsystem.h"
#include "DEquipment.h"

DEFINE_LOG_CATEGORY_STATIC(LogItemSubsystem, Log, All);

void UItemSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogItemSubsystem, Log, TEXT("UItemSubsystem Initialized"));
}

void UItemSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UItemSubsystem::AddEquip(UDEquipment* Equip)
{
	if (!Equip) return;
	EquipList.Add(Equip);
	UE_LOG(LogItemSubsystem, Log, TEXT("AddEquip: Id=%d (Total: %d)"), Equip->EquipmentId, EquipList.Num());
}

void UItemSubsystem::RemoveEquip(UDEquipment* Equip)
{
	if (!Equip) return;
	EquipList.Remove(Equip);
	UE_LOG(LogItemSubsystem, Log, TEXT("RemoveEquip: Id=%d (Total: %d)"), Equip->EquipmentId, EquipList.Num());
}

void UItemSubsystem::ResetAll()
{
	EquipList.Empty();
	UE_LOG(LogItemSubsystem, Log, TEXT("ResetAll"));
}
