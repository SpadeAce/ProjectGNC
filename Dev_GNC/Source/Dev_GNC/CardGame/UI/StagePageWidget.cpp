// Copyright GNC Project. All Rights Reserved.

#include "StagePageWidget.h"
#include "BattleCardWidget.h"
#include "CardDragDropOp.h"
#include "CardGameHUD.h"
#include "DCard.h"
#include "DPawn.h"
#include "DObject.h"
#include "CardGameActor.h"
#include "GridSlotComponent.h"
#include "SquareTile.h"
#include "CardGameRowTypes.h"
#include "StageSubsystem.h"
#include "TurnSubsystem.h"
#include "DeckSubsystem.h"
#include "PausePopupWidget.h"
#include "BattleResultPopupWidget.h"
#include "Components/TextBlock.h"
#include "Components/PanelWidget.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

DEFINE_LOG_CATEGORY_STATIC(LogStagePage, Log, All);

// ────────────────────────────────────────────────────────────
// 라이프사이클
// ────────────────────────────────────────────────────────────

void UStagePageWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	UWorld* World = GetWorld();
	if (!World) return;

	TurnSubRef = World->GetSubsystem<UTurnSubsystem>();
	StageSubRef = World->GetSubsystem<UStageSubsystem>();
	DeckSubRef = GetGameInstance()->GetSubsystem<UDeckSubsystem>();

	// 델리게이트 구독
	if (UDeckSubsystem* DeckSub = DeckSubRef.Get())
	{
		DeckSub->OnHandChanged.AddDynamic(this, &UStagePageWidget::HandleHandChanged);
	}
	if (UStageSubsystem* StageSub = StageSubRef.Get())
	{
		StageSub->OnBattleEnd.AddDynamic(this, &UStagePageWidget::HandleBattleEnd);
		StageSub->OnSelectedChanged.AddDynamic(this, &UStagePageWidget::HandleSelectedChanged);
	}
	if (UTurnSubsystem* TurnSub = TurnSubRef.Get())
	{
		TurnSub->OnTurnChanged.AddDynamic(this, &UStagePageWidget::HandleTurnChanged);
		TurnSub->OnActingPowerChanged.AddDynamic(this, &UStagePageWidget::HandleActingPowerChanged);
	}

	// 버튼 바인딩
	if (ButtonPause)
	{
		ButtonPause->OnClicked.AddDynamic(this, &UStagePageWidget::HandlePauseClicked);
	}
	if (ButtonEndTurn)
	{
		ButtonEndTurn->OnClicked.AddDynamic(this, &UStagePageWidget::HandleEndTurnClicked);
	}
}

void UStagePageWidget::NativeOnOpened()
{
	Super::NativeOnOpened();

	// 초기 UI 갱신
	if (UTurnSubsystem* TurnSub = TurnSubRef.Get())
	{
		HandleTurnChanged(TurnSub->CurrentTurn);
		HandleActingPowerChanged();
	}
	HandleHandChanged();
	HandleSelectedChanged();
}

void UStagePageWidget::NativePreClose()
{
	// 델리게이트 해제
	if (UDeckSubsystem* DeckSub = DeckSubRef.Get())
	{
		DeckSub->OnHandChanged.RemoveDynamic(this, &UStagePageWidget::HandleHandChanged);
	}
	if (UStageSubsystem* StageSub = StageSubRef.Get())
	{
		StageSub->OnBattleEnd.RemoveDynamic(this, &UStagePageWidget::HandleBattleEnd);
		StageSub->OnSelectedChanged.RemoveDynamic(this, &UStagePageWidget::HandleSelectedChanged);
	}
	if (UTurnSubsystem* TurnSub = TurnSubRef.Get())
	{
		TurnSub->OnTurnChanged.RemoveDynamic(this, &UStagePageWidget::HandleTurnChanged);
		TurnSub->OnActingPowerChanged.RemoveDynamic(this, &UStagePageWidget::HandleActingPowerChanged);
	}

	if (ButtonPause) ButtonPause->OnClicked.RemoveAll(this);
	if (ButtonEndTurn) ButtonEndTurn->OnClicked.RemoveAll(this);

	UnsubscribePawnHand();

	Super::NativePreClose();
}

// ────────────────────────────────────────────────────────────
// 핸드 카드 관리
// ────────────────────────────────────────────────────────────

void UStagePageWidget::HandleHandChanged()
{
	UDeckSubsystem* DeckSub = DeckSubRef.Get();
	if (!DeckSub) return;

	SyncCardWidgets(DeckSub->HandCardList, CardWidgetPool, CardGroupPanel, nullptr);
	RefreshCardStates();
}

void UStagePageWidget::HandleSelectedChanged()
{
	UStageSubsystem* StageSub = StageSubRef.Get();
	if (!StageSub) return;

	// 이전 폰 구독 해제
	UnsubscribePawnHand();

	UDPawn* SelectedPawn = Cast<UDPawn>(StageSub->SelectedObject);
	if (SelectedPawn)
	{
		SubscribedPawn = SelectedPawn;
		SelectedPawn->OnPawnHandChanged.AddDynamic(this, &UStagePageWidget::HandlePawnHandChanged);

		if (PawnCardGroupPanel)
		{
			PawnCardGroupPanel->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
		SetPawnCardList();
	}
	else
	{
		// 선택 해제 또는 몬스터 선택 — 폰 카드 패널 숨김
		if (PawnCardGroupPanel)
		{
			PawnCardGroupPanel->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// 글로벌 카드의 CasterActor도 갱신
	RefreshCardStates();
}

void UStagePageWidget::HandlePawnHandChanged()
{
	SetPawnCardList();
	RefreshCardStates();
}

void UStagePageWidget::SetPawnCardList()
{
	UDPawn* Pawn = SubscribedPawn.Get();
	if (!Pawn) return;

	SyncCardWidgets(Pawn->PawnHandCards, PawnCardWidgetPool, PawnCardGroupPanel, Pawn);
}

void UStagePageWidget::SyncCardWidgets(
	const TArray<TObjectPtr<UDCard>>& Cards,
	TArray<TObjectPtr<UBattleCardWidget>>& Pool,
	UPanelWidget* Panel,
	UDPawn* Owner)
{
	if (!Panel || !BattleCardWidgetClass) return;

	const int32 CardCount = Cards.Num();

	// 부족하면 생성
	while (Pool.Num() < CardCount)
	{
		UBattleCardWidget* Widget = CreateWidget<UBattleCardWidget>(this, BattleCardWidgetClass);
		if (Widget)
		{
			Panel->AddChild(Widget);
			Pool.Add(Widget);
		}
	}

	// 카드 데이터 설정
	for (int32 i = 0; i < Pool.Num(); ++i)
	{
		UBattleCardWidget* Widget = Pool[i];
		if (!Widget) continue;

		if (i < CardCount)
		{
			Widget->SetData(Cards[i], Owner);
			Widget->SetCardVisible(true);

			// CasterActor 설정
			ACardGameActor* Caster = nullptr;
			if (Owner)
			{
				Caster = FindActorForPawn(Owner);
			}
			else
			{
				// 글로벌 카드: 현재 선택된 폰의 액터
				if (UDPawn* SelPawn = SubscribedPawn.Get())
				{
					Caster = FindActorForPawn(SelPawn);
				}
			}
			Widget->SetCasterActor(Caster);
		}
		else
		{
			Widget->SetCardVisible(false);
		}
	}
}

void UStagePageWidget::RefreshCardStates()
{
	UTurnSubsystem* TurnSub = TurnSubRef.Get();
	if (!TurnSub) return;

	const bool bMonsterTurn = TurnSub->IsMonsterTurn();

	auto RefreshPool = [&](TArray<TObjectPtr<UBattleCardWidget>>& Pool)
	{
		for (UBattleCardWidget* Widget : Pool)
		{
			if (!Widget || Widget->GetVisibility() == ESlateVisibility::Collapsed) continue;

			UDCard* Card = Widget->GetCard();
			if (!Card || !Card->Data) continue;

			bool bDisabled = bMonsterTurn;

			if (!bDisabled)
			{
				bDisabled = !TurnSub->HasEnoughSharedActingPower(Card->Data->EnergyCost);
			}

			if (!bDisabled)
			{
				if (UDPawn* Owner = Widget->GetCardOwner())
				{
					bDisabled = !Owner->HasEnoughAmmo(Card->Data->AmmoCost);
				}
			}

			Widget->SetDisabled(bDisabled);
		}
	};

	RefreshPool(CardWidgetPool);
	RefreshPool(PawnCardWidgetPool);
}

ACardGameActor* UStagePageWidget::FindActorForPawn(UDPawn* Pawn) const
{
	UStageSubsystem* StageSub = StageSubRef.Get();
	if (!StageSub || !Pawn) return nullptr;

	for (const auto& Pair : StageSub->UserActors)
	{
		if (Pair.Value && Pair.Value->Data == Pawn)
		{
			return Pair.Value;
		}
	}
	return nullptr;
}

void UStagePageWidget::UnsubscribePawnHand()
{
	if (UDPawn* Pawn = SubscribedPawn.Get())
	{
		Pawn->OnPawnHandChanged.RemoveDynamic(this, &UStagePageWidget::HandlePawnHandChanged);
	}
	SubscribedPawn = nullptr;
}

// ────────────────────────────────────────────────────────────
// UI 갱신 핸들러
// ────────────────────────────────────────────────────────────

void UStagePageWidget::HandleTurnChanged(int32 NewTurn)
{
	if (TextTurn)
	{
		TextTurn->SetText(FText::AsNumber(NewTurn));
	}
}

void UStagePageWidget::HandleActingPowerChanged()
{
	if (UTurnSubsystem* TurnSub = TurnSubRef.Get())
	{
		if (TextEnergy)
		{
			TextEnergy->SetText(FText::AsNumber(TurnSub->SharedActingPower));
		}
	}

	RefreshCardStates();
}

void UStagePageWidget::HandleBattleEnd(bool bVictory)
{
	ACardGameHUD* HUD = OwningHUD.Get();
	if (!HUD) return;

	FViewParam Param;
	Param.bFlag = bVictory;
	Param.IntValue = StageSubRef.IsValid() ? StageSubRef->StageGoldReward : 0;

	HUD->OpenPopup(BattleResultPopupClass, Param);

	UE_LOG(LogStagePage, Log, TEXT("BattleEnd: %s — opened BattleResultPopup"), bVictory ? TEXT("Victory") : TEXT("Defeat"));
}

void UStagePageWidget::HandlePauseClicked()
{
	ACardGameHUD* HUD = OwningHUD.Get();
	if (!HUD) return;

	HUD->OpenPopup(PausePopupClass);

	UE_LOG(LogStagePage, Log, TEXT("Pause clicked — opened PausePopup"));
}

void UStagePageWidget::HandleEndTurnClicked()
{
	if (UTurnSubsystem* TurnSub = TurnSubRef.Get())
	{
		TurnSub->EndPawnTurn();
	}
}

// ────────────────────────────────────────────────────────────
// 드래그-드롭
// ────────────────────────────────────────────────────────────

void UStagePageWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);

	UCardDragDropOp* DragOp = Cast<UCardDragDropOp>(InOperation);
	if (!DragOp || !DragOp->Card || !DragOp->CasterActor) return;

	UStageSubsystem* StageSub = StageSubRef.Get();
	if (!StageSub) return;

	// 캐스터 타일에서 카드 범위 표시
	ACardGameActor* Caster = DragOp->CasterActor;
	if (Caster && Caster->CurrentSlot)
	{
		ASquareTile* CasterTile = Cast<ASquareTile>(Caster->CurrentSlot->GetOwner());
		if (CasterTile)
		{
			StageSub->ShowCardRange(CasterTile, DragOp->Card, Caster);
		}
	}
}

void UStagePageWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);

	if (UStageSubsystem* StageSub = StageSubRef.Get())
	{
		StageSub->ClearCardRange();
		StageSub->ClearRadiusPreview();
	}
}

bool UStagePageWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	UCardDragDropOp* DragOp = Cast<UCardDragDropOp>(InOperation);
	if (!DragOp || !DragOp->Card || !DragOp->CasterActor) return false;

	UStageSubsystem* StageSub = StageSubRef.Get();
	if (!StageSub) return false;

	UDCard* Card = DragOp->Card;
	ACardGameActor* Caster = DragOp->CasterActor;
	UDPawn* CardOwner = DragOp->CardOwner;

	const ETargetType Target = Card->Data ? Card->Data->Target : ETargetType::None;

	bool bUsed = false;

	if (Target == ETargetType::Self)
	{
		// 자기 자신에게 사용
		StageSub->UseCardSelf(Card, Caster, CardOwner);
		bUsed = true;
	}
	else
	{
		// 커서 아래 월드 히트로 타일 탐색
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		FHitResult Hit;
		if (PC && PC->GetHitResultUnderCursorByChannel(UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
		{
			ASquareTile* TargetTile = Cast<ASquareTile>(Hit.GetActor());
			if (TargetTile && StageSub->IsInCardRange(TargetTile))
			{
				bUsed = StageSub->TryUseCard(Card, Caster, TargetTile, CardOwner);
			}
		}
	}

	// 범위 표시 정리
	StageSub->ClearCardRange();
	StageSub->ClearRadiusPreview();

	if (!bUsed)
	{
		UE_LOG(LogStagePage, Log, TEXT("Card drop cancelled or invalid target"));
	}

	return true;
}
