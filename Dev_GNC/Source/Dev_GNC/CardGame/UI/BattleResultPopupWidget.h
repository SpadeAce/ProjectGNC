// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PopupWidget.h"
#include "BattleResultPopupWidget.generated.h"

class UButton;
class UTextBlock;
class UPanelWidget;
class UDCard;

/**
 * UBattleResultPopupWidget
 * 전투 종료 후 결과(승리/패배) + 보상 카드 선택 팝업.
 * Unity BattleResultPopup.cs 대응.
 *
 * FViewParam 매핑:
 *  - bFlag     = bVictory
 *  - IntValue  = StageGoldReward
 *
 * StageExpReward는 NativePreOpen에서 StageSubsystem으로부터 직접 조회.
 */
UCLASS(Abstract, BlueprintType)
class UBattleResultPopupWidget : public UPopupWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativePreClose() override;

protected:
	// ── BindWidget ──────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextRewardGold;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextRewardExp;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonContinue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonExit;

	/** 보상 카드 선택 영역. 승리 시만 표시. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> RewardCardPanel;

	/** 보상 카드 버튼 3개. BP에서 BindWidget으로 매핑. */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonRewardCard0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonRewardCard1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonRewardCard2;

	/** 보상 카드 이름 텍스트 3개 */
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextRewardCard0;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextRewardCard1;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextRewardCard2;

private:
	// ── 상태 ────────────────────────────────────────
	bool bVictory = false;
	int32 GoldReward = 0;
	int32 ExpReward = 0;

	static constexpr int32 RewardCardCount = 3;
	TArray<int32> RewardCardIds;
	int32 SelectedCardIndex = -1;

	// ── 핸들러 ──────────────────────────────────────
	UFUNCTION() void HandleContinueClicked();
	UFUNCTION() void HandleExitClicked();
	UFUNCTION() void HandleRewardCard0Clicked();
	UFUNCTION() void HandleRewardCard1Clicked();
	UFUNCTION() void HandleRewardCard2Clicked();

	// ── 내부 헬퍼 ───────────────────────────────────
	void SetupRewardCards();
	TArray<int32> PickWeightedCards(const TArray<int32>& CardIds, const TArray<int32>& CardProbs, int32 Count);
	void SelectRewardCard(int32 Index);
	void UpdateCardSelectionVisual();

	UButton* GetRewardButton(int32 Index) const;
	UTextBlock* GetRewardText(int32 Index) const;
};
