// Copyright GNC Project. All Rights Reserved.

#include "ShopCardItemWidget.h"
#include "DCard.h"
#include "CardGameRowTypes.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UShopCardItemWidget::SetData(UDCard* InCard)
{
	Card = InCard;

	if (!InCard || !InCard->Data)
	{
		if (TextName) TextName->SetText(FText::GetEmpty());
		if (TextPrice) TextPrice->SetText(FText::GetEmpty());
		return;
	}

	if (TextName)
	{
		TextName->SetText(FText::FromString(InCard->Data->IdAlias));
	}
	if (TextPrice)
	{
		TextPrice->SetText(FText::AsNumber(InCard->Data->Price));
	}
}

void UShopCardItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonBuy)
	{
		ButtonBuy->OnClicked.AddDynamic(this, &UShopCardItemWidget::HandleBuyClicked);
	}
}

void UShopCardItemWidget::NativeDestruct()
{
	if (ButtonBuy)
	{
		ButtonBuy->OnClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UShopCardItemWidget::HandleBuyClicked()
{
	if (Card)
	{
		OnBuyRequested.Broadcast(Card);
	}
}
