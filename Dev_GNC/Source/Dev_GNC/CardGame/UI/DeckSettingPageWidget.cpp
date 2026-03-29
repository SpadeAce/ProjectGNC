// Copyright GNC Project. All Rights Reserved.

#include "DeckSettingPageWidget.h"
#include "PawnListItemWidget.h"
#include "CardGameHUD.h"
#include "DPawn.h"
#include "DCard.h"
#include "CardGameRowTypes.h"
#include "DeckSubsystem.h"
#include "PawnSubsystem.h"
#include "PlayerDataSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogDeckSettingPage, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void UDeckSettingPageWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	// 서브시스템 캐시
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		DeckSubRef   = GI->GetSubsystem<UDeckSubsystem>();
		PawnSubRef   = GI->GetSubsystem<UPawnSubsystem>();
		PlayerSubRef = GI->GetSubsystem<UPlayerDataSubsystem>();
	}

	// 슬롯 배열 초기화
	SlotButtons = { ButtonSlot0, ButtonSlot1, ButtonSlot2, ButtonSlot3, ButtonSlot4 };
	SlotTexts   = { TextSlot0, TextSlot1, TextSlot2, TextSlot3, TextSlot4 };

	// 버튼 바인딩
	if (ButtonPawnTab) ButtonPawnTab->OnClicked.AddDynamic(this, &UDeckSettingPageWidget::HandlePawnTabClicked);
	if (ButtonCardTab) ButtonCardTab->OnClicked.AddDynamic(this, &UDeckSettingPageWidget::HandleCardTabClicked);
	if (ButtonClose)   ButtonClose->OnClicked.AddDynamic(this, &UDeckSettingPageWidget::HandleCloseClicked);

	if (ButtonSlot0) ButtonSlot0->OnClicked.AddDynamic(this, &UDeckSettingPageWidget::HandleSlot0Clicked);
	if (ButtonSlot1) ButtonSlot1->OnClicked.AddDynamic(this, &UDeckSettingPageWidget::HandleSlot1Clicked);
	if (ButtonSlot2) ButtonSlot2->OnClicked.AddDynamic(this, &UDeckSettingPageWidget::HandleSlot2Clicked);
	if (ButtonSlot3) ButtonSlot3->OnClicked.AddDynamic(this, &UDeckSettingPageWidget::HandleSlot3Clicked);
	if (ButtonSlot4) ButtonSlot4->OnClicked.AddDynamic(this, &UDeckSettingPageWidget::HandleSlot4Clicked);
}

void UDeckSettingPageWidget::NativeOnOpened()
{
	Super::NativeOnOpened();
	SelectedPawn = nullptr;
	ShowPawnTab();
}

void UDeckSettingPageWidget::NativePreClose()
{
	if (ButtonPawnTab) ButtonPawnTab->OnClicked.RemoveAll(this);
	if (ButtonCardTab) ButtonCardTab->OnClicked.RemoveAll(this);
	if (ButtonClose)   ButtonClose->OnClicked.RemoveAll(this);

	for (UButton* Btn : SlotButtons)
	{
		if (Btn) Btn->OnClicked.RemoveAll(this);
	}

	Super::NativePreClose();
}

// ────────────────────────────────────────────────────────────
// Tab
// ────────────────────────────────────────────────────────────

void UDeckSettingPageWidget::HandlePawnTabClicked()  { ShowPawnTab(); }
void UDeckSettingPageWidget::HandleCardTabClicked()  { ShowCardTab(); }
void UDeckSettingPageWidget::HandleCloseClicked()    { RequestClose(); }

void UDeckSettingPageWidget::ShowPawnTab()
{
	if (PanelPawnTab) PanelPawnTab->SetVisibility(ESlateVisibility::Visible);
	if (PanelCardTab) PanelCardTab->SetVisibility(ESlateVisibility::Collapsed);
	RefreshSlots();
	RefreshOwnedPawnList();
}

void UDeckSettingPageWidget::ShowCardTab()
{
	if (PanelPawnTab) PanelPawnTab->SetVisibility(ESlateVisibility::Collapsed);
	if (PanelCardTab) PanelCardTab->SetVisibility(ESlateVisibility::Visible);
	RefreshCardList();
}

// ────────────────────────────────────────────────────────────
// Slot Handlers
// ────────────────────────────────────────────────────────────

void UDeckSettingPageWidget::HandleSlot0Clicked() { HandleSlotClicked(0); }
void UDeckSettingPageWidget::HandleSlot1Clicked() { HandleSlotClicked(1); }
void UDeckSettingPageWidget::HandleSlot2Clicked() { HandleSlotClicked(2); }
void UDeckSettingPageWidget::HandleSlot3Clicked() { HandleSlotClicked(3); }
void UDeckSettingPageWidget::HandleSlot4Clicked() { HandleSlotClicked(4); }

void UDeckSettingPageWidget::HandleSlotClicked(int32 SlotIndex)
{
	UDeckSubsystem* DeckSub = DeckSubRef.Get();
	if (!DeckSub) return;

	// 잠긴 슬롯: 골드 소모 후 확장
	if (SlotIndex >= DeckSub->OpenSlotCount)
	{
		UPlayerDataSubsystem* PlayerSub = PlayerSubRef.Get();
		if (!PlayerSub || !PlayerSub->SpendGold(ExpandSlotCost))
		{
			UE_LOG(LogDeckSettingPage, Warning, TEXT("슬롯 확장 실패: 골드 부족"));
			return;
		}
		DeckSub->ExpandSlot();
		UE_LOG(LogDeckSettingPage, Log, TEXT("슬롯 확장 → %d/%d"), DeckSub->OpenSlotCount, SlotCount);
		RefreshSlots();
		RefreshOwnedPawnList();
		return;
	}

	// 폰이 있는 슬롯: 해제
	UDPawn* SlotPawn = DeckSub->GetDeckPawn(SlotIndex);
	if (SlotPawn)
	{
		DeckSub->UnassignPawnFromDeck(SlotIndex);
		RefreshSlots();
		RefreshOwnedPawnList();
		return;
	}

	// 빈 슬롯 + 선택된 폰: 배치
	if (SelectedPawn.IsValid())
	{
		DeckSub->AssignPawnToDeck(SelectedPawn.Get(), SlotIndex);
		SelectedPawn = nullptr;
		RefreshSlots();
		RefreshOwnedPawnList();
	}
}

// ────────────────────────────────────────────────────────────
// Refresh
// ────────────────────────────────────────────────────────────

void UDeckSettingPageWidget::RefreshSlots()
{
	UDeckSubsystem* DeckSub = DeckSubRef.Get();
	if (!DeckSub) return;

	for (int32 i = 0; i < SlotCount; ++i)
	{
		UTextBlock* Text = (i < SlotTexts.Num()) ? SlotTexts[i] : nullptr;
		if (!Text) continue;

		if (i >= DeckSub->OpenSlotCount)
		{
			Text->SetText(FText::FromString(FString::Printf(TEXT("LOCKED (%dG)"), ExpandSlotCost)));
		}
		else
		{
			UDPawn* Pawn = DeckSub->GetDeckPawn(i);
			Text->SetText(Pawn
				? FText::FromString(Pawn->CodeName)
				: FText::FromString(TEXT("(Empty)")));
		}
	}
}

void UDeckSettingPageWidget::RefreshOwnedPawnList()
{
	if (!ScrollOwnedPawns) return;
	ScrollOwnedPawns->ClearChildren();

	UDeckSubsystem* DeckSub = DeckSubRef.Get();
	UPawnSubsystem* PawnSub = PawnSubRef.Get();
	if (!DeckSub || !PawnSub || !PawnListItemClass) return;

	// 편성된 폰 InstanceId 집합 구성
	TSet<int64> AssignedIds;
	for (int32 i = 0; i < DeckSub->OpenSlotCount; ++i)
	{
		if (UDPawn* P = DeckSub->GetDeckPawn(i))
		{
			AssignedIds.Add(P->InstanceId);
		}
	}

	// 편성되지 않은 폰만 표시
	for (UDPawn* Pawn : PawnSub->GetPawns())
	{
		if (!Pawn || AssignedIds.Contains(Pawn->InstanceId)) continue;

		UPawnListItemWidget* Item = CreateWidget<UPawnListItemWidget>(this, PawnListItemClass);
		if (!Item) continue;

		Item->SetData(Pawn);
		Item->OnItemClicked.AddDynamic(this, &UDeckSettingPageWidget::HandleOwnedPawnSelected);
		ScrollOwnedPawns->AddChild(Item);
	}
}

void UDeckSettingPageWidget::RefreshCardList()
{
	if (!ScrollDeckCards) return;
	ScrollDeckCards->ClearChildren();

	UDeckSubsystem* DeckSub = DeckSubRef.Get();
	if (!DeckSub) return;

	for (UDCard* Card : DeckSub->GetDeckCards())
	{
		if (!Card || !Card->Data) continue;

		UTextBlock* Text = NewObject<UTextBlock>(ScrollDeckCards);
		Text->SetText(FText::FromString(FString::Printf(TEXT("[%d] %s"), Card->CardId, *Card->Data->IdAlias)));
		ScrollDeckCards->AddChild(Text);
	}
}

void UDeckSettingPageWidget::HandleOwnedPawnSelected(UDPawn* InPawn)
{
	SelectedPawn = InPawn;
	UE_LOG(LogDeckSettingPage, Log, TEXT("Pawn selected: %s"), InPawn ? *InPawn->CodeName : TEXT("null"));
}
