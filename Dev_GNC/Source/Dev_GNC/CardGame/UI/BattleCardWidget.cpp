// Copyright GNC Project. All Rights Reserved.

#include "BattleCardWidget.h"
#include "CardDragDropOp.h"
#include "DCard.h"
#include "DPawn.h"
#include "CardGameRowTypes.h"
#include "TextSubsystem.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UBattleCardWidget::SetData(UDCard* InCard, UDPawn* InOwner)
{
	Card = InCard;
	CardOwner = InOwner;

	if (!Card || !Card->Data)
	{
		SetCardVisible(false);
		return;
	}

	const FCardDataRow* Data = Card->Data;

	// 로컬라이즈된 이름/설명
	UTextSubsystem* TextSub = GetGameInstance()->GetSubsystem<UTextSubsystem>();
	const FString CardName = TextSub ? TextSub->Get(Data->NameAlias) : Data->NameAlias;
	const FString CardDesc = TextSub ? TextSub->Get(Data->DescAlias) : Data->DescAlias;

	if (TextEnergyCost)
	{
		TextEnergyCost->SetText(FText::AsNumber(Data->EnergyCost));
	}
	if (TextAmmoCost)
	{
		TextAmmoCost->SetText(FText::AsNumber(Data->AmmoCost));
	}
	if (TextName)
	{
		TextName->SetText(FText::FromString(CardName));
	}
	if (TextDesc)
	{
		TextDesc->SetText(FText::FromString(CardDesc));
	}

	// 아이콘은 BP에서 IconPath 기반으로 설정하거나 여기서 로드
	// Phase 6 에셋 연결 전까지는 빈 상태 유지

	SetCardVisible(true);
}

void UBattleCardWidget::SetDisabled(bool bDisabled)
{
	if (ImageDimmed)
	{
		ImageDimmed->SetVisibility(bDisabled ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
	}
}

void UBattleCardWidget::SetCasterActor(ACardGameActor* InActor)
{
	CasterActor = InActor;
}

void UBattleCardWidget::SetCardVisible(bool bVisible)
{
	SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

FReply UBattleCardWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && Card)
	{
		return UWidgetBlueprintLibrary::DetectDragIfPressed(InMouseEvent, this, EKeys::LeftMouseButton).NativeReply;
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void UBattleCardWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
	UCardDragDropOp* DragOp = NewObject<UCardDragDropOp>();
	DragOp->Card = Card;
	DragOp->CardOwner = CardOwner;
	DragOp->CasterActor = CasterActor.Get();
	DragOp->DefaultDragVisual = this;
	DragOp->Pivot = EDragPivot::CenterCenter;

	OutOperation = DragOp;
}
