// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PawnListItemWidget.generated.h"

class UDPawn;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnItemClicked, UDPawn*, Pawn);

/**
 * UPawnListItemWidget
 * 폰 목록의 개별 항목 위젯. DeckSettingPage / PawnManagePage 공용.
 * Unity DeckPawnItem / PawnManageItem 대응.
 */
UCLASS(Abstract, BlueprintType)
class UPawnListItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void SetData(UDPawn* InPawn);

	UFUNCTION(BlueprintPure, Category = "CardGame|UI")
	UDPawn* GetPawn() const { return Pawn; }

	UPROPERTY(BlueprintAssignable, Category = "CardGame|UI")
	FOnPawnItemClicked OnItemClicked;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonSelect;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TObjectPtr<UDPawn> Pawn;

	UFUNCTION()
	void HandleSelectClicked();
};
