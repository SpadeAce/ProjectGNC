// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PageWidget.h"
#include "DeckSettingPageWidget.generated.h"

class UButton;
class UTextBlock;
class UPanelWidget;
class UScrollBox;
class UDPawn;
class UDeckSubsystem;
class UPawnSubsystem;
class UPlayerDataSubsystem;

/**
 * UDeckSettingPageWidget
 * 덱 편성 페이지. Pawn 탭 (슬롯 편성) / Card 탭 (카드 열람) 전환.
 * Unity DeckSettingPage 대응.
 */
UCLASS(Abstract, BlueprintType)
class UDeckSettingPageWidget : public UPageWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativeOnOpened() override;
	virtual void NativePreClose() override;

protected:
	// ── 탭 / 닫기 ───────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonPawnTab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonCardTab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonClose;

	// ── Pawn 탭 ─────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PanelPawnTab;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonSlot0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonSlot1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonSlot2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonSlot3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonSlot4;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextSlot0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextSlot1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextSlot2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextSlot3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextSlot4;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollOwnedPawns;

	// ── Card 탭 ─────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PanelCardTab;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollDeckCards;

	// ── 에디터 설정 ─────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UUserWidget> PawnListItemClass;

private:
	static constexpr int32 SlotCount = 5;
	static constexpr int32 ExpandSlotCost = 100;

	// 서브시스템 캐시
	TWeakObjectPtr<UDeckSubsystem> DeckSubRef;
	TWeakObjectPtr<UPawnSubsystem> PawnSubRef;
	TWeakObjectPtr<UPlayerDataSubsystem> PlayerSubRef;

	// 슬롯 배열 (NativePreOpen에서 초기화)
	TArray<UButton*> SlotButtons;
	TArray<UTextBlock*> SlotTexts;

	// 선택 상태
	TWeakObjectPtr<UDPawn> SelectedPawn;

	// ── 탭 전환 ─────────────────────────────────────
	UFUNCTION() void HandlePawnTabClicked();
	UFUNCTION() void HandleCardTabClicked();
	UFUNCTION() void HandleCloseClicked();

	void ShowPawnTab();
	void ShowCardTab();

	// ── 슬롯 핸들러 ────────────────────────────────
	UFUNCTION() void HandleSlot0Clicked();
	UFUNCTION() void HandleSlot1Clicked();
	UFUNCTION() void HandleSlot2Clicked();
	UFUNCTION() void HandleSlot3Clicked();
	UFUNCTION() void HandleSlot4Clicked();
	void HandleSlotClicked(int32 SlotIndex);

	// ── 갱신 ────────────────────────────────────────
	void RefreshSlots();
	void RefreshOwnedPawnList();
	void RefreshCardList();

	UFUNCTION()
	void HandleOwnedPawnSelected(UDPawn* InPawn);
};
