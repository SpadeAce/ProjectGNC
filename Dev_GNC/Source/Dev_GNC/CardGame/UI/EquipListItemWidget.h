// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "EquipListItemWidget.generated.h"

class UDEquipment;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipItemRightClicked, UDEquipment*, Equipment);

/**
 * UEquipListItemWidget
 * 미장착 장비 목록의 개별 항목 위젯. PawnManagePage용.
 * Unity PawnManageEquipItem 대응.
 */
UCLASS(Abstract, BlueprintType)
class UEquipListItemWidget : public UUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void SetData(UDEquipment* InEquip);

	UFUNCTION(BlueprintPure, Category = "CardGame|UI")
	UDEquipment* GetEquip() const { return Equip; }

	UPROPERTY(BlueprintAssignable, Category = "CardGame|UI")
	FOnEquipItemRightClicked OnItemRightClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextSlotType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonSelect;

	virtual void NativeOnListItemObjectSet(UObject* ListItemObject) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual FReply NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	UPROPERTY()
	TObjectPtr<UDEquipment> Equip;

	bool bRightMouseDown = false;

	UFUNCTION()
	void HandleDragCancelled(UDragDropOperation* Operation);
};
