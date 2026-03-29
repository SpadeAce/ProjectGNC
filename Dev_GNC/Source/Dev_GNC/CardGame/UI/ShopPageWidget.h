// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PageWidget.h"
#include "ShopPageWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UDCard;
class ULobbySubsystem;
class UPlayerDataSubsystem;
class UDeckSubsystem;

/**
 * UShopPageWidget
 * 상점 페이지. 카드 구매 + 상점 레벨업.
 * Unity ShopPage 대응.
 */
UCLASS(Abstract, BlueprintType)
class UShopPageWidget : public UPageWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativeOnOpened() override;
	virtual void NativePreClose() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonClose;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonLevelUp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextGold;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextShopLevel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollShopCards;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UUserWidget> ShopCardItemClass;

private:
	static constexpr int32 LevelUpCost = 50;

	TWeakObjectPtr<ULobbySubsystem> LobbySubRef;
	TWeakObjectPtr<UPlayerDataSubsystem> PlayerSubRef;
	TWeakObjectPtr<UDeckSubsystem> DeckSubRef;

	UFUNCTION() void HandleCloseClicked();
	UFUNCTION() void HandleLevelUpClicked();
	UFUNCTION() void HandleBuyCard(UDCard* Card);
	UFUNCTION() void HandleShopListChanged();

	void RefreshShopDisplay();
	void RefreshGold();
};
