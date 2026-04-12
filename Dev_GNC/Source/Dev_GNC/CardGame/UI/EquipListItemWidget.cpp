// Copyright GNC Project. All Rights Reserved.

#include "EquipListItemWidget.h"
#include "EquipDragDropOp.h"
#include "DEquipment.h"
#include "CardGameRowTypes.h"
#include "TextSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

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
		UTextSubsystem* TextSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTextSubsystem>() : nullptr;
		const FString Resolved = TextSub ? TextSub->Get(InEquip->Data->NameAlias) : InEquip->Data->NameAlias;
		TextName->SetText(FText::FromString(Resolved));
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

void UEquipListItemWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);
	SetData(Cast<UDEquipment>(ListItemObject));
}

FReply UEquipListItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && Equip)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		bRightMouseDown = true;
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UEquipListItemWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::RightMouseButton && bRightMouseDown)
	{
		bRightMouseDown = false;
		if (Equip)
		{
			OnItemRightClicked.Broadcast(Equip);
		}
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UEquipListItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	// 드래그 비주얼용 위젯을 별도 생성 (BP 서브클래스 그대로 복제)
	UEquipListItemWidget* DragVisual = CreateWidget<UEquipListItemWidget>(this, GetClass());
	if (DragVisual)
	{
		DragVisual->SetData(Equip);
	}

	UEquipDragDropOp* DragOp = NewObject<UEquipDragDropOp>();
	DragOp->Equipment = Equip;
	DragOp->DefaultDragVisual = DragVisual;
	DragOp->Pivot = EDragPivot::CenterCenter;
	DragOp->OnDragCancelled.AddDynamic(this, &UEquipListItemWidget::HandleDragCancelled);

	// 원본 아이템을 리스트에서 숨김
	SetVisibility(ESlateVisibility::Hidden);

	OutOperation = DragOp;
}

void UEquipListItemWidget::HandleDragCancelled(UDragDropOperation* Operation)
{
	SetVisibility(ESlateVisibility::Visible);
}
