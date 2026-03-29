// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PageWidget.h"
#include "RecruitPageWidget.generated.h"

class UButton;
class UTextBlock;
class UPanelWidget;
class UDPawn;
class ULobbySubsystem;
class UPawnSubsystem;
class UPlayerDataSubsystem;

/**
 * URecruitPageWidget
 * 모집 페이지. 3개 폰 슬롯 + 리롤 기능.
 * Unity RecruitPage 대응.
 */
UCLASS(Abstract, BlueprintType)
class URecruitPageWidget : public UPageWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativeOnOpened() override;
	virtual void NativePreClose() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonClose;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonReroll;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextGold;

	// ── 3개 고정 슬롯 ──────────────────────────────
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPanelWidget> PanelRecruit0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPanelWidget> PanelRecruit1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UPanelWidget> PanelRecruit2;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonRecruit0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonRecruit1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonRecruit2;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextRecruitName0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextRecruitName1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextRecruitName2;

private:
	static constexpr int32 RecruitSlotCount = 3;
	static constexpr int32 RecruitCost = 100;
	static constexpr int32 RerollCost = 50;

	TWeakObjectPtr<ULobbySubsystem> LobbySubRef;
	TWeakObjectPtr<UPawnSubsystem> PawnSubRef;
	TWeakObjectPtr<UPlayerDataSubsystem> PlayerSubRef;

	TArray<UPanelWidget*> RecruitPanels;
	TArray<UButton*> RecruitButtons;
	TArray<UTextBlock*> RecruitTexts;

	UFUNCTION() void HandleCloseClicked();
	UFUNCTION() void HandleRerollClicked();

	UFUNCTION() void HandleRecruit0Clicked();
	UFUNCTION() void HandleRecruit1Clicked();
	UFUNCTION() void HandleRecruit2Clicked();
	void HandleRecruitClicked(int32 Index);

	UFUNCTION() void HandleRecruitListChanged();

	void RefreshRecruitDisplay();
	void RefreshGold();
};
