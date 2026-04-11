// Copyright GNC Project. All Rights Reserved.

#include "PawnListItemWidget.h"
#include "DPawn.h"
#include "TextSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void UPawnListItemWidget::SetData(UDPawn* InPawn)
{
	Pawn = InPawn;

	if (TextName)
	{
		if (InPawn)
		{
			UTextSubsystem* TextSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTextSubsystem>() : nullptr;
			const FString Resolved = TextSub ? TextSub->Get(InPawn->CodeName) : InPawn->CodeName;
			TextName->SetText(FText::FromString(Resolved));
		}
		else
		{
			TextName->SetText(FText::GetEmpty());
		}
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
