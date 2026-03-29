// Copyright GNC Project. All Rights Reserved.

#include "PawnListItemWidget.h"
#include "DPawn.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UPawnListItemWidget::SetData(UDPawn* InPawn)
{
	Pawn = InPawn;

	if (TextName)
	{
		TextName->SetText(InPawn ? FText::FromString(InPawn->CodeName) : FText::GetEmpty());
	}
}

void UPawnListItemWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ButtonSelect)
	{
		ButtonSelect->OnClicked.AddDynamic(this, &UPawnListItemWidget::HandleSelectClicked);
	}
}

void UPawnListItemWidget::NativeDestruct()
{
	if (ButtonSelect)
	{
		ButtonSelect->OnClicked.RemoveAll(this);
	}

	Super::NativeDestruct();
}

void UPawnListItemWidget::HandleSelectClicked()
{
	if (Pawn)
	{
		OnItemClicked.Broadcast(Pawn);
	}
}
