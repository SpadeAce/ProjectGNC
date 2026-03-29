// Copyright GNC Project. All Rights Reserved.

#include "EquipListItemWidget.h"
#include "DEquipment.h"
#include "CardGameRowTypes.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UEquipListItemWidget::SetData(UDEquipment* InEquip)
{
	Equip = InEquip;

	if (!InEquip || !InEquip->Data)
	{
		if (TextName) TextName->SetText(FText::GetEmpty());
		if (TextSlotType) TextSlotType->SetText(FText::GetEmpty());
		return;
	}

	if (TextName)
	{
		TextName->SetText(FText::FromString(InEquip->Data->IdAlias));
	}
	if (TextSlotType)
	{
		static const TMap<EEquipSlotType, FString> SlotNames = {
			{ EEquipSlotType::WeaponMain, TEXT("Weapon") },
			{ EEquipSlotType::WeaponSub,  TEXT("SubWeapon") },
			{ EEquipSlotType::Armor,      TEXT("Armor") },
			{ EEquipSlotType::Helmet,     TEXT("Helmet") },
			{ EEquipSlotType::Tool,       TEXT("Tool") },
		};
		const FString* Found = SlotNames.Find(InEquip->Data->Slot);
		TextSlotType->SetText(FText::FromString(Found ? *Found : TEXT("None")));
	}
}

void UEquipListItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonSelect)
	{
		ButtonSelect->OnClicked.AddDynamic(this, &UEquipListItemWidget::HandleSelectClicked);
	}
}

void UEquipListItemWidget::NativeDestruct()
{
	if (ButtonSelect)
	{
		ButtonSelect->OnClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UEquipListItemWidget::HandleSelectClicked()
{
	if (Equip)
	{
		OnItemClicked.Broadcast(Equip);
	}
}
