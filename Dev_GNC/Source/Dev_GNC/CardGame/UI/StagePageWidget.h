// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PageWidget.h"
#include "StagePageWidget.generated.h"

class UBattleCardWidget;
class UDPawn;
class UDCard;
class ACardGameActor;
class UTurnSubsystem;
class UStageSubsystem;
class UDeckSubsystem;
class UTextBlock;
class UPanelWidget;
class UButton;
class UPausePopupWidget;
class UBattleResultPopupWidget;

/**
 * UStagePageWidget
 * 전투 중 메인 HUD 페이지. 카드 핸드, 턴/에너지, 드래그-드롭 처리.
 * Unity StagePage.cs 대응.
 */
UCLASS(Abstract, BlueprintType)
class UStagePageWidget : public UPageWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativeOnOpened() override;
	virtual void NativePreClose() override;

protected:
	// ── BindWidget ──────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextTurn;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextEnergy;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> CardGroupPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PawnCardGroupPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonPause;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonEndTurn;

	// ── 에디터 설정 ─────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UBattleCardWidget> BattleCardWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UPausePopupWidget> PausePopupClass;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UBattleResultPopupWidget> BattleResultPopupClass;

	// ── 드래그-드롭 ─────────────────────────────────
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;
	virtual void NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

private:
	// ── 서브시스템 캐시 ─────────────────────────────
	TWeakObjectPtr<UTurnSubsystem> TurnSubRef;
	TWeakObjectPtr<UStageSubsystem> StageSubRef;
	TWeakObjectPtr<UDeckSubsystem> DeckSubRef;

	// ── 카드 위젯 풀 ───────────────────────────────
	UPROPERTY()
	TArray<TObjectPtr<UBattleCardWidget>> CardWidgetPool;

	UPROPERTY()
	TArray<TObjectPtr<UBattleCardWidget>> PawnCardWidgetPool;

	TWeakObjectPtr<UDPawn> SubscribedPawn;

	// ── 핸들러 ──────────────────────────────────────
	UFUNCTION() void HandleHandChanged();
	UFUNCTION() void HandleBattleEnd(bool bVictory);
	UFUNCTION() void HandleSelectedChanged();
	UFUNCTION() void HandleTurnChanged(int32 NewTurn);
	UFUNCTION() void HandleActingPowerChanged();
	UFUNCTION() void HandlePawnHandChanged();
	UFUNCTION() void HandlePauseClicked();
	UFUNCTION() void HandleEndTurnClicked();

	// ── 내부 헬퍼 ───────────────────────────────────
	void SetPawnCardList();
	void RefreshCardStates();
	void SyncCardWidgets(const TArray<TObjectPtr<UDCard>>& Cards, TArray<TObjectPtr<UBattleCardWidget>>& Pool, UPanelWidget* Panel, UDPawn* Owner);
	ACardGameActor* FindActorForPawn(UDPawn* Pawn) const;
	void UnsubscribePawnHand();
};
