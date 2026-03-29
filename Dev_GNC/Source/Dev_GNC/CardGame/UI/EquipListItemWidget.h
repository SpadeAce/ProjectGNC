// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EquipListItemWidget.generated.h"

class UDEquipment;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEquipItemClicked, UDEquipment*, Equipment);

/**
 * UEquipListItemWidget
 * 미장착 장비 목록의 개별 항목 위젯. PawnManagePage용.
 * Unity PawnManageEquipItem 대응.
 */
UCLASS(Abstract, BlueprintType)
class UEquipListItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void SetData(UDEquipment* InEquip);

	UFUNCTION(BlueprintPure, Category = "CardGame|UI")
	UDEquipment* GetEquip() const { return Equip; }

	UPROPERTY(BlueprintAssignable, Category = "CardGame|UI")
	FOnEquipItemClicked OnItemClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextSlotType;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonSelect;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TObjectPtr<UDEquipment> Equip;

	UFUNCTION()
	void HandleSelectClicked();
};
