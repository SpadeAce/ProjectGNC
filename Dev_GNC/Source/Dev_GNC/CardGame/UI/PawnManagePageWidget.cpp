// Copyright GNC Project. All Rights Reserved.

#include "PawnManagePageWidget.h"
#include "PawnListItemWidget.h"
#include "EquipListItemWidget.h"
#include "EquipDragDropOp.h"
#include "CardGameHUD.h"
#include "DPawn.h"
#include "DEquipment.h"
#include "CardGameRowTypes.h"
#include "PawnSubsystem.h"
#include "ItemSubsystem.h"
#include "TextSubsystem.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/ScrollBox.h"
#include "Components/TileView.h"

DEFINE_LOG_CATEGORY_STATIC(LogPawnManagePage, Log, All);

const EEquipSlotType UPawnManagePageWidget::SlotTypeMap[EquipSlotCount] = {
	EEquipSlotType::WeaponMain,
	EEquipSlotType::Armor,
	EEquipSlotType::Tool,
	EEquipSlotType::Tool,
};

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void UPawnManagePageWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		PawnSubRef = GI->GetSubsystem<UPawnSubsystem>();
		ItemSubRef = GI->GetSubsystem<UItemSubsystem>();
	}

	EquipSlotButtons = { ButtonEquipSlot0, ButtonEquipSlot1, ButtonEquipSlot2, ButtonEquipSlot3 };
	EquipSlotTexts   = { TextEquipSlot0, TextEquipSlot1, TextEquipSlot2, TextEquipSlot3 };

	if (ButtonClose) ButtonClose->OnClicked.AddDynamic(this, &UPawnManagePageWidget::HandleCloseClicked);

	if (ButtonEquipSlot0) ButtonEquipSlot0->OnClicked.AddDynamic(this, &UPawnManagePageWidget::HandleEquipSlot0Clicked);
	if (ButtonEquipSlot1) ButtonEquipSlot1->OnClicked.AddDynamic(this, &UPawnManagePageWidget::HandleEquipSlot1Clicked);
	if (ButtonEquipSlot2) ButtonEquipSlot2->OnClicked.AddDynamic(this, &UPawnManagePageWidget::HandleEquipSlot2Clicked);
	if (ButtonEquipSlot3) ButtonEquipSlot3->OnClicked.AddDynamic(this, &UPawnManagePageWidget::HandleEquipSlot3Clicked);

	if (TileUnequippedList)
	{
		TileUnequippedList->OnEntryWidgetGenerated().AddUObject(this, &UPawnManagePageWidget::HandleEntryWidgetGenerated);
		TileUnequippedList->OnEntryWidgetReleased().AddUObject(this, &UPawnManagePageWidget::HandleEntryWidgetReleased);
	}
}

void UPawnManagePageWidget::NativeOnOpened()
{
	Super::NativeOnOpened();

	SelectedPawn = nullptr;
	if (PanelPawnInfo) PanelPawnInfo->SetVisibility(ESlateVisibility::Collapsed);

	RefreshPawnList();
	RefreshUnequippedList();
}

void UPawnManagePageWidget::NativePreClose()
{
	// 선택된 폰의 스탯 델리게이트 해제
	if (UDPawn* Pawn = SelectedPawn.Get())
	{
		Pawn->OnStatsChanged.RemoveDynamic(this, &UPawnManagePageWidget::HandleStatsChanged);
	}

	if (ButtonClose) ButtonClose->OnClicked.RemoveAll(this);
	for (UButton* Btn : EquipSlotButtons)
	{
		if (Btn) Btn->OnClicked.RemoveAll(this);
	}
	if (TileUnequippedList)
	{
		TileUnequippedList->OnEntryWidgetGenerated().RemoveAll(this);
		TileUnequippedList->OnEntryWidgetReleased().RemoveAll(this);
	}

	Super::NativePreClose();
}

// ────────────────────────────────────────────────────────────
// Handlers
// ────────────────────────────────────────────────────────────

void UPawnManagePageWidget::HandleCloseClicked() { RequestClose(); }

void UPawnManagePageWidget::HandleEquipSlot0Clicked() { HandleEquipSlotClicked(0); }
void UPawnManagePageWidget::HandleEquipSlot1Clicked() { HandleEquipSlotClicked(1); }
void UPawnManagePageWidget::HandleEquipSlot2Clicked() { HandleEquipSlotClicked(2); }
void UPawnManagePageWidget::HandleEquipSlot3Clicked() { HandleEquipSlotClicked(3); }

void UPawnManagePageWidget::HandleEquipSlotClicked(int32 SlotIndex)
{
	UDPawn* Pawn = SelectedPawn.Get();
	if (!Pawn) return;

	// 슬롯에 장비가 있으면 해제
	UDEquipment* SlotEquip = FindEquipInSlot(SlotIndex);
	if (SlotEquip)
	{
		Pawn->Unequip(SlotEquip);
		RefreshEquipSlots();
		RefreshUnequippedList();
		RefreshPawnStats();
	}
}

void UPawnManagePageWidget::HandlePawnSelected(UDPawn* InPawn)
{
	// 이전 폰 스탯 구독 해제
	if (UDPawn* PrevPawn = SelectedPawn.Get())
	{
		PrevPawn->OnStatsChanged.RemoveDynamic(this, &UPawnManagePageWidget::HandleStatsChanged);
	}

	SelectedPawn = InPawn;

	if (InPawn)
	{
		InPawn->OnStatsChanged.AddDynamic(this, &UPawnManagePageWidget::HandleStatsChanged);
	}

	if (PanelPawnInfo)
	{
		PanelPawnInfo->SetVisibility(InPawn ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	RefreshPawnStats();
	RefreshEquipSlots();
	RefreshUnequippedList();
}

void UPawnManagePageWidget::HandleEntryWidgetGenerated(UUserWidget& Widget)
{
	if (UEquipListItemWidget* EquipWidget = Cast<UEquipListItemWidget>(&Widget))
	{
		EquipWidget->OnItemRightClicked.AddDynamic(this, &UPawnManagePageWidget::HandleEquipItemRightClicked);
	}
}

void UPawnManagePageWidget::HandleEntryWidgetReleased(UUserWidget& Widget)
{
	if (UEquipListItemWidget* EquipWidget = Cast<UEquipListItemWidget>(&Widget))
	{
		EquipWidget->OnItemRightClicked.RemoveDynamic(this, &UPawnManagePageWidget::HandleEquipItemRightClicked);
	}
}

void UPawnManagePageWidget::HandleEquipItemRightClicked(UDEquipment* InEquip)
{
	if (!SelectedPawn.Get() || !InEquip || !InEquip->Data) return;

	AutoEquipToSlot(InEquip);
}

void UPawnManagePageWidget::AutoEquipToSlot(UDEquipment* InEquip)
{
	UDPawn* Pawn = SelectedPawn.Get();
	if (!Pawn || !InEquip || !InEquip->Data) return;

	const EEquipSlotType TargetSlot = InEquip->Data->Slot;

	// 매칭되는 슬롯 인덱스 수집
	TArray<int32> MatchingSlots;
	for (int32 i = 0; i < EquipSlotCount; ++i)
	{
		if (SlotTypeMap[i] == TargetSlot)
		{
			MatchingSlots.Add(i);
		}
	}

	if (MatchingSlots.IsEmpty()) return;

	// 빈 슬롯 우선 탐색
	int32 TargetIndex = -1;
	for (int32 SlotIdx : MatchingSlots)
	{
		if (FindEquipInSlot(SlotIdx) == nullptr)
		{
			TargetIndex = SlotIdx;
			break;
		}
	}

	// 빈 슬롯 없으면 첫 번째 매칭 슬롯 (기존 장비 교체)
	if (TargetIndex < 0)
	{
		TargetIndex = MatchingSlots[0];
		UDEquipment* OldEquip = FindEquipInSlot(TargetIndex);
		if (OldEquip)
		{
			Pawn->Unequip(OldEquip);
		}
	}

	// 장착
	Pawn->Equip(InEquip);

	// UI 갱신
	RefreshEquipSlots();
	RefreshUnequippedList();
	RefreshPawnStats();
}

bool UPawnManagePageWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UEquipDragDropOp* DragOp = Cast<UEquipDragDropOp>(InOperation);
	if (!DragOp || !DragOp->Equipment || !DragOp->Equipment->Data) return false;

	UDPawn* Pawn = SelectedPawn.Get();
	if (!Pawn) return false;

	const FVector2D ScreenPos = InDragDropEvent.GetScreenSpacePosition();

	for (int32 i = 0; i < EquipSlotCount; ++i)
	{
		UButton* SlotBtn = (i < EquipSlotButtons.Num()) ? EquipSlotButtons[i] : nullptr;
		if (!SlotBtn) continue;

		const FGeometry SlotGeo = SlotBtn->GetCachedGeometry();
		const FVector2D LocalPos = SlotGeo.AbsoluteToLocal(ScreenPos);
		const FVector2D Size = SlotGeo.GetLocalSize();

		if (LocalPos.X >= 0.f && LocalPos.Y >= 0.f && LocalPos.X <= Size.X && LocalPos.Y <= Size.Y)
		{
			// 슬롯 타입 일치 확인
			if (DragOp->Equipment->Data->Slot != SlotTypeMap[i]) break;

			// 기존 장비 해제
			UDEquipment* OldEquip = FindEquipInSlot(i);
			if (OldEquip)
			{
				Pawn->Unequip(OldEquip);
			}

			// 장착
			Pawn->Equip(DragOp->Equipment);

			RefreshEquipSlots();
			RefreshUnequippedList();
			RefreshPawnStats();
			return true;
		}
	}

	return false;
}

void UPawnManagePageWidget::HandleStatsChanged()
{
	RefreshPawnStats();
}

// ────────────────────────────────────────────────────────────
// Refresh
// ────────────────────────────────────────────────────────────

void UPawnManagePageWidget::RefreshPawnList()
{
	if (!ScrollPawnList) return;
	ScrollPawnList->ClearChildren();

	UPawnSubsystem* PawnSub = PawnSubRef.Get();
	if (!PawnSub || !PawnListItemClass) return;

	for (UDPawn* Pawn : PawnSub->GetPawns())
	{
		if (!Pawn) continue;

		UPawnListItemWidget* Item = CreateWidget<UPawnListItemWidget>(this, PawnListItemClass);
		if (!Item) continue;

		Item->SetData(Pawn);
		Item->OnItemClicked.AddDynamic(this, &UPawnManagePageWidget::HandlePawnSelected);
		ScrollPawnList->AddChild(Item);
	}
}

void UPawnManagePageWidget::RefreshPawnStats()
{
	UDPawn* Pawn = SelectedPawn.Get();
	if (!Pawn)
	{
		if (TextPawnName) TextPawnName->SetText(FText::GetEmpty());
		return;
	}

	if (TextPawnName)
	{
		UTextSubsystem* TextSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTextSubsystem>() : nullptr;
		const FString Resolved = TextSub ? TextSub->Get(Pawn->CodeName) : Pawn->CodeName;
		TextPawnName->SetText(FText::FromString(Resolved));
	}
	if (TextHP)       TextHP->SetText(FText::FromString(FString::Printf(TEXT("HP: %d"), Pawn->HP)));
	if (TextATK)      TextATK->SetText(FText::FromString(FString::Printf(TEXT("ATK: %d"), Pawn->Attack)));
	if (TextDEF)      TextDEF->SetText(FText::FromString(FString::Printf(TEXT("DEF: %d"), Pawn->Armor)));
	if (TextShield)   TextShield->SetText(FText::FromString(FString::Printf(TEXT("SHIELD: %d"), Pawn->Shield)));
	if (TextMovement) TextMovement->SetText(FText::FromString(FString::Printf(TEXT("MOVE: %d"), Pawn->Movement)));
}

void UPawnManagePageWidget::RefreshEquipSlots()
{
	for (int32 i = 0; i < EquipSlotCount; ++i)
	{
		UTextBlock* Text = (i < EquipSlotTexts.Num()) ? EquipSlotTexts[i] : nullptr;
		if (!Text) continue;

		UDEquipment* Equip = FindEquipInSlot(i);
		if (Equip && Equip->Data)
		{
			Text->SetText(FText::FromString(Equip->Data->IdAlias));
		}
		else
		{
			static const FString SlotLabels[] = { TEXT("Weapon"), TEXT("Armor"), TEXT("Tool 1"), TEXT("Tool 2") };
			Text->SetText(FText::FromString(FString::Printf(TEXT("(%s)"), *SlotLabels[i])));
		}
	}
}

void UPawnManagePageWidget::RefreshUnequippedList()
{
	if (!TileUnequippedList) return;

	UItemSubsystem* ItemSub = ItemSubRef.Get();
	if (!ItemSub)
	{
		TileUnequippedList->ClearListItems();
		return;
	}

	TArray<UObject*> Items;
	for (UDEquipment* Equip : ItemSub->GetEquips())
	{
		if (!Equip || Equip->IsEquipped()) continue;
		Items.Add(Equip);
	}
	TileUnequippedList->SetListItems(Items);
}

UDEquipment* UPawnManagePageWidget::FindEquipInSlot(int32 SlotIndex) const
{
	UDPawn* Pawn = SelectedPawn.Get();
	if (!Pawn || SlotIndex < 0 || SlotIndex >= EquipSlotCount) return nullptr;

	const EEquipSlotType TargetType = SlotTypeMap[SlotIndex];

	// Tool 슬롯은 2개 (인덱스 2,3). 해당 타입의 N번째 장비를 반환
	int32 TypeOccurrence = 0;
	int32 DesiredOccurrence = 0;
	for (int32 i = 0; i < SlotIndex; ++i)
	{
		if (SlotTypeMap[i] == TargetType) ++DesiredOccurrence;
	}

	for (UDEquipment* Equip : Pawn->Equips)
	{
		if (Equip && Equip->Data && Equip->Data->Slot == TargetType)
		{
			if (TypeOccurrence == DesiredOccurrence) return Equip;
			++TypeOccurrence;
		}
	}

	return nullptr;
}
