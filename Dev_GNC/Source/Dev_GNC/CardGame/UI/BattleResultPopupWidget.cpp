// Copyright GNC Project. All Rights Reserved.

#include "BattleResultPopupWidget.h"
#include "CardGameHUD.h"
#include "CardGameRowTypes.h"
#include "DCard.h"
#include "DPawn.h"
#include "DataSubsystem.h"
#include "DeckSubsystem.h"
#include "PlayerDataSubsystem.h"
#include "PawnSubsystem.h"
#include "ItemSubsystem.h"
#include "LobbySubsystem.h"
#include "StageSubsystem.h"
#include "TextSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogBattleResultPopup, Log, All);

// ────────────────────────────────────────────────────────────
// 라이프사이클
// ────────────────────────────────────────────────────────────

void UBattleResultPopupWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	// Param 해석
	bVictory = Param.bFlag;
	GoldReward = Param.IntValue;

	// ExpReward는 StageSubsystem에서 직접 조회
	if (UWorld* World = GetWorld())
	{
		if (UStageSubsystem* StageSub = World->GetSubsystem<UStageSubsystem>())
		{
			ExpReward = StageSub->StageExpReward;
		}
	}

	// UI 설정
	if (TextTitle)
	{
		TextTitle->SetText(bVictory ? FText::FromString(TEXT("승리")) : FText::FromString(TEXT("패배")));
	}
	if (TextRewardGold)
	{
		TextRewardGold->SetText(FText::AsNumber(GoldReward));
	}
	if (TextRewardExp)
	{
		TextRewardExp->SetText(FText::AsNumber(ExpReward));
	}

	// 승리/패배에 따른 버튼 표시
	if (ButtonContinue)
	{
		ButtonContinue->SetVisibility(bVictory ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
	if (ButtonExit)
	{
		ButtonExit->SetVisibility(bVictory ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}

	// 보상 카드 영역
	SelectedCardIndex = -1;
	RewardCardIds.Empty();

	if (bVictory)
	{
		if (RewardCardPanel) RewardCardPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		SetupRewardCards();
	}
	else
	{
		if (RewardCardPanel) RewardCardPanel->SetVisibility(ESlateVisibility::Collapsed);
	}

	// 버튼 바인딩
	if (ButtonContinue) ButtonContinue->OnClicked.AddDynamic(this, &UBattleResultPopupWidget::HandleContinueClicked);
	if (ButtonExit)     ButtonExit->OnClicked.AddDynamic(this, &UBattleResultPopupWidget::HandleExitClicked);
	if (ButtonRewardCard0) ButtonRewardCard0->OnClicked.AddDynamic(this, &UBattleResultPopupWidget::HandleRewardCard0Clicked);
	if (ButtonRewardCard1) ButtonRewardCard1->OnClicked.AddDynamic(this, &UBattleResultPopupWidget::HandleRewardCard1Clicked);
	if (ButtonRewardCard2) ButtonRewardCard2->OnClicked.AddDynamic(this, &UBattleResultPopupWidget::HandleRewardCard2Clicked);
}

void UBattleResultPopupWidget::NativePreClose()
{
	if (ButtonContinue) ButtonContinue->OnClicked.RemoveAll(this);
	if (ButtonExit)     ButtonExit->OnClicked.RemoveAll(this);
	if (ButtonRewardCard0) ButtonRewardCard0->OnClicked.RemoveAll(this);
	if (ButtonRewardCard1) ButtonRewardCard1->OnClicked.RemoveAll(this);
	if (ButtonRewardCard2) ButtonRewardCard2->OnClicked.RemoveAll(this);

	Super::NativePreClose();
}

// ────────────────────────────────────────────────────────────
// 보상 카드 설정
// ────────────────────────────────────────────────────────────

void UBattleResultPopupWidget::SetupRewardCards()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UDataSubsystem* DataSub = GI->GetSubsystem<UDataSubsystem>();
	UPlayerDataSubsystem* PlayerSub = GI->GetSubsystem<UPlayerDataSubsystem>();
	if (!DataSub || !PlayerSub) return;

	const FStageRewardRow* RewardRow = DataSub->GetStageRewardByLevel(PlayerSub->DifficultyLevel);
	if (!RewardRow || RewardRow->CardId.Num() == 0)
	{
		if (RewardCardPanel) RewardCardPanel->SetVisibility(ESlateVisibility::Collapsed);
		UE_LOG(LogBattleResultPopup, Log, TEXT("No reward cards for difficulty level %d"), PlayerSub->DifficultyLevel);
		return;
	}

	RewardCardIds = PickWeightedCards(RewardRow->CardId, RewardRow->CardProb, RewardCardCount);

	// 카드 버튼에 이름 표시
	for (int32 i = 0; i < RewardCardCount; ++i)
	{
		UButton* Btn = GetRewardButton(i);
		UTextBlock* Txt = GetRewardText(i);

		if (i < RewardCardIds.Num())
		{
			if (Btn) Btn->SetVisibility(ESlateVisibility::Visible);

			if (Txt)
			{
				const FCardDataRow* CardRow = DataSub->GetCardData(RewardCardIds[i]);
				if (CardRow)
				{
					UTextSubsystem* TextSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTextSubsystem>() : nullptr;
					const FString Resolved = TextSub ? TextSub->Get(CardRow->NameAlias) : CardRow->NameAlias;
					Txt->SetText(FText::FromString(Resolved));
				}
				else
				{
					Txt->SetText(FText::FromString(TEXT("???")));
				}
			}
		}
		else
		{
			if (Btn) Btn->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	UpdateCardSelectionVisual();
}

TArray<int32> UBattleResultPopupWidget::PickWeightedCards(
	const TArray<int32>& CardIds,
	const TArray<int32>& CardProbs,
	int32 Count)
{
	TArray<int32> Result;
	TArray<int32> Candidates;
	TArray<int32> Weights;

	for (int32 i = 0; i < CardIds.Num(); ++i)
	{
		Candidates.Add(i);
		Weights.Add(i < CardProbs.Num() ? CardProbs[i] : 1);
	}

	const int32 Pick = FMath::Min(Count, Candidates.Num());

	for (int32 n = 0; n < Pick; ++n)
	{
		int32 TotalWeight = 0;
		for (int32 W : Weights) TotalWeight += W;

		if (TotalWeight <= 0) break;

		int32 Roll = FMath::RandRange(0, TotalWeight - 1);
		int32 Cumulative = 0;
		int32 SelectedIdx = 0;

		for (int32 i = 0; i < Weights.Num(); ++i)
		{
			Cumulative += Weights[i];
			if (Roll < Cumulative)
			{
				SelectedIdx = i;
				break;
			}
		}

		Result.Add(CardIds[Candidates[SelectedIdx]]);
		Candidates.RemoveAt(SelectedIdx);
		Weights.RemoveAt(SelectedIdx);
	}

	return Result;
}

// ────────────────────────────────────────────────────────────
// 카드 선택
// ────────────────────────────────────────────────────────────

void UBattleResultPopupWidget::SelectRewardCard(int32 Index)
{
	if (SelectedCardIndex == Index)
	{
		// 토글 해제
		SelectedCardIndex = -1;
	}
	else
	{
		SelectedCardIndex = Index;
	}

	UpdateCardSelectionVisual();
}

void UBattleResultPopupWidget::UpdateCardSelectionVisual()
{
	for (int32 i = 0; i < RewardCardCount; ++i)
	{
		UButton* Btn = GetRewardButton(i);
		if (!Btn) continue;

		// 선택된 카드는 Normal 스타일, 비선택은 기본 스타일 유지
		// BP에서 WidgetStyle로 시각 피드백 세팅.
		// C++에서는 RenderOpacity로 간단 표시.
		Btn->SetRenderOpacity(i == SelectedCardIndex ? 1.0f : 0.6f);
	}
}

UButton* UBattleResultPopupWidget::GetRewardButton(int32 Index) const
{
	switch (Index)
	{
	case 0: return ButtonRewardCard0;
	case 1: return ButtonRewardCard1;
	case 2: return ButtonRewardCard2;
	default: return nullptr;
	}
}

UTextBlock* UBattleResultPopupWidget::GetRewardText(int32 Index) const
{
	switch (Index)
	{
	case 0: return TextRewardCard0;
	case 1: return TextRewardCard1;
	case 2: return TextRewardCard2;
	default: return nullptr;
	}
}

// ────────────────────────────────────────────────────────────
// 버튼 핸들러
// ────────────────────────────────────────────────────────────

void UBattleResultPopupWidget::HandleRewardCard0Clicked() { SelectRewardCard(0); }
void UBattleResultPopupWidget::HandleRewardCard1Clicked() { SelectRewardCard(1); }
void UBattleResultPopupWidget::HandleRewardCard2Clicked() { SelectRewardCard(2); }

void UBattleResultPopupWidget::HandleContinueClicked()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UPlayerDataSubsystem* PlayerSub = GI->GetSubsystem<UPlayerDataSubsystem>();
	UDeckSubsystem* DeckSub = GI->GetSubsystem<UDeckSubsystem>();

	// 골드 보상
	if (PlayerSub)
	{
		PlayerSub->AddGold(GoldReward);
		PlayerSub->IncrementDifficulty();
	}

	// 경험치 보상 — 편성 폰 전원
	if (DeckSub)
	{
		TArray<UDPawn*> ActivePawns = DeckSub->GetActivePawns();
		for (UDPawn* Pawn : ActivePawns)
		{
			if (Pawn)
			{
				Pawn->AddExp(ExpReward);
			}
		}
	}

	// 선택된 보상 카드 추가
	if (DeckSub && SelectedCardIndex >= 0 && SelectedCardIndex < RewardCardIds.Num())
	{
		UDCard* NewCard = NewObject<UDCard>(GI);
		NewCard->InitFromId(RewardCardIds[SelectedCardIndex], GI->GetSubsystem<UDataSubsystem>());
		DeckSub->AddCard(NewCard);

		UE_LOG(LogBattleResultPopup, Log, TEXT("Reward card added: Id=%d"), RewardCardIds[SelectedCardIndex]);
	}

	UE_LOG(LogBattleResultPopup, Log, TEXT("Continue: Gold=%d, Exp=%d, CardIdx=%d"), GoldReward, ExpReward, SelectedCardIndex);
	UGameplayStatics::OpenLevel(this, FName("Lobby"));
}

void UBattleResultPopupWidget::HandleExitClicked()
{
	// 패배 시 서브시스템 리셋 후 타이틀로 복귀
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (auto* Sub = GI->GetSubsystem<UPlayerDataSubsystem>()) Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UPawnSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UDeckSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UItemSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<ULobbySubsystem>())      Sub->ResetAll();
	}

	UE_LOG(LogBattleResultPopup, Log, TEXT("Exit: returning to title"));
	UGameplayStatics::OpenLevel(this, FName("Title"));
}
