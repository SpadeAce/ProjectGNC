// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopCardItemWidget.generated.h"

class UDCard;
class UTextBlock;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnShopBuyRequested, UDCard*, Card);

/**
 * UShopCardItemWidget
 * 상점 카드 1장을 표시하고 구매 버튼을 제공하는 위젯.
 * Unity ShopItem 대응.
 */
UCLASS(Abstract, BlueprintType)
class UShopCardItemWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void SetData(UDCard* InCard);

	UFUNCTION(BlueprintPure, Category = "CardGame|UI")
	UDCard* GetCard() const { return Card; }

	UPROPERTY(BlueprintAssignable, Category = "CardGame|UI")
	FOnShopBuyRequested OnBuyRequested;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextPrice;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonBuy;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	TObjectPtr<UDCard> Card;

	UFUNCTION()
	void HandleBuyClicked();
};
