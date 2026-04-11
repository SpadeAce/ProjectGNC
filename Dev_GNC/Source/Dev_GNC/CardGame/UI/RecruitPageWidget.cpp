// Copyright GNC Project. All Rights Reserved.

#include "RecruitPageWidget.h"
#include "CardGameHUD.h"
#include "DPawn.h"
#include "LobbySubsystem.h"
#include "PawnSubsystem.h"
#include "PlayerDataSubsystem.h"
#include "TextSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogRecruitPage, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void URecruitPageWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		LobbySubRef  = GI->GetSubsystem<ULobbySubsystem>();
		PawnSubRef   = GI->GetSubsystem<UPawnSubsystem>();
		PlayerSubRef = GI->GetSubsystem<UPlayerDataSubsystem>();
	}

	RecruitPanels  = { PanelRecruit0, PanelRecruit1, PanelRecruit2 };
	RecruitButtons = { ButtonRecruit0, ButtonRecruit1, ButtonRecruit2 };
	RecruitTexts   = { TextRecruitName0, TextRecruitName1, TextRecruitName2 };

	if (ButtonClose)  ButtonClose->OnClicked.AddDynamic(this, &URecruitPageWidget::HandleCloseClicked);
	if (ButtonReroll) ButtonReroll->OnClicked.AddDynamic(this, &URecruitPageWidget::HandleRerollClicked);

	if (ButtonRecruit0) ButtonRecruit0->OnClicked.AddDynamic(this, &URecruitPageWidget::HandleRecruit0Clicked);
	if (ButtonRecruit1) ButtonRecruit1->OnClicked.AddDynamic(this, &URecruitPageWidget::HandleRecruit1Clicked);
	if (ButtonRecruit2) ButtonRecruit2->OnClicked.AddDynamic(this, &URecruitPageWidget::HandleRecruit2Clicked);

	if (ULobbySubsystem* LobbySub = LobbySubRef.Get())
	{
		LobbySub->OnRecruitListChanged.AddDynamic(this, &URecruitPageWidget::HandleRecruitListChanged);
	}
}

void URecruitPageWidget::NativeOnOpened()
{
	Super::NativeOnOpened();

	RefreshRecruitDisplay();
	RefreshGold();
}

void URecruitPageWidget::NativePreClose()
{
	if (ButtonClose)  ButtonClose->OnClicked.RemoveAll(this);
	if (ButtonReroll) ButtonReroll->OnClicked.RemoveAll(this);

	for (UButton* Btn : RecruitButtons)
	{
		if (Btn) Btn->OnClicked.RemoveAll(this);
	}

	if (ULobbySubsystem* LobbySub = LobbySubRef.Get())
	{
		LobbySub->OnRecruitListChanged.RemoveDynamic(this, &URecruitPageWidget::HandleRecruitListChanged);
	}

	Super::NativePreClose();
}

// ────────────────────────────────────────────────────────────
// Handlers
// ────────────────────────────────────────────────────────────

void URecruitPageWidget::HandleCloseClicked() { RequestClose(); }

void URecruitPageWidget::HandleRerollClicked()
{
	ULobbySubsystem* LobbySub = LobbySubRef.Get();
	UPlayerDataSubsystem* PlayerSub = PlayerSubRef.Get();
	if (!LobbySub || !PlayerSub) return;

	if (!PlayerSub->SpendGold(RerollCost))
	{
		UE_LOG(LogRecruitPage, Warning, TEXT("리롤 실패: 골드 부족"));
		return;
	}

	LobbySub->GenerateRecruitList();
	RefreshRecruitDisplay();
	RefreshGold();
}

void URecruitPageWidget::HandleRecruit0Clicked() { HandleRecruitClicked(0); }
void URecruitPageWidget::HandleRecruit1Clicked() { HandleRecruitClicked(1); }
void URecruitPageWidget::HandleRecruit2Clicked() { HandleRecruitClicked(2); }

void URecruitPageWidget::HandleRecruitClicked(int32 Index)
{
	ULobbySubsystem* LobbySub = LobbySubRef.Get();
	UPawnSubsystem* PawnSub = PawnSubRef.Get();
	UPlayerDataSubsystem* PlayerSub = PlayerSubRef.Get();
	if (!LobbySub || !PawnSub || !PlayerSub) return;

	if (Index < 0 || Index >= LobbySub->RecruitPawns.Num()) return;

	UDPawn* Pawn = LobbySub->RecruitPawns[Index];
	if (!Pawn) return;

	if (!PlayerSub->SpendGold(RecruitCost))
	{
		UE_LOG(LogRecruitPage, Warning, TEXT("모집 실패: 골드 부족"));
		return;
	}

	PawnSub->AddPawn(Pawn);
	LobbySub->RemoveRecruitPawn(Pawn);

	RefreshRecruitDisplay();
	RefreshGold();

	UE_LOG(LogRecruitPage, Log, TEXT("폰 모집: %s (-%dG)"), *Pawn->CodeName, RecruitCost);
}

void URecruitPageWidget::HandleRecruitListChanged()
{
	RefreshRecruitDisplay();
}

// ────────────────────────────────────────────────────────────
// Refresh
// ────────────────────────────────────────────────────────────

void URecruitPageWidget::RefreshRecruitDisplay()
{
	ULobbySubsystem* LobbySub = LobbySubRef.Get();

	for (int32 i = 0; i < RecruitSlotCount; ++i)
	{
		UPanelWidget* Panel = (i < RecruitPanels.Num()) ? RecruitPanels[i] : nullptr;
		UTextBlock* Text    = (i < RecruitTexts.Num()) ? RecruitTexts[i] : nullptr;

		const bool bHasPawn = LobbySub && i < LobbySub->RecruitPawns.Num() && LobbySub->RecruitPawns[i] != nullptr;

		if (Panel)
		{
			Panel->SetVisibility(bHasPawn ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}

		if (Text && bHasPawn)
		{
			UTextSubsystem* TextSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTextSubsystem>() : nullptr;
			const FString& CodeName = LobbySub->RecruitPawns[i]->CodeName;
			const FString Resolved = TextSub ? TextSub->Get(CodeName) : CodeName;
			Text->SetText(FText::FromString(Resolved));
		}
	}
}

void URecruitPageWidget::RefreshGold()
{
	if (TextGold && PlayerSubRef.IsValid())
	{
		TextGold->SetText(FText::AsNumber(PlayerSubRef->Gold));
	}
}
