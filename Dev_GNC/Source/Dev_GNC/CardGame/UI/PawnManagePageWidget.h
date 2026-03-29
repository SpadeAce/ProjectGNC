// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PageWidget.h"
#include "CardGameTypes.h"
#include "PawnManagePageWidget.generated.h"

class UButton;
class UTextBlock;
class UScrollBox;
class UPanelWidget;
class UDPawn;
class UDEquipment;
class UPawnSubsystem;
class UItemSubsystem;

/**
 * UPawnManagePageWidget
 * 폰 관리 페이지. 좌측 폰 목록, 우측 스탯+장비 슬롯 4개, 하단 미장착 장비 목록.
 * Unity PawnManagePage 대응.
 */
UCLASS(Abstract, BlueprintType)
class UPawnManagePageWidget : public UPageWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativeOnOpened() override;
	virtual void NativePreClose() override;

protected:
	// ── 닫기 ────────────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonClose;

	// ── 좌측: 폰 목록 ──────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollPawnList;

	// ── 우측: 스탯 정보 ─────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> PanelPawnInfo;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextPawnName;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextHP;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextATK;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextDEF;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextShield;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextMovement;

	// ── 장비 슬롯 4개 ──────────────────────────────
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonEquipSlot0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonEquipSlot1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonEquipSlot2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UButton> ButtonEquipSlot3;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextEquipSlot0;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextEquipSlot1;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextEquipSlot2;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UTextBlock> TextEquipSlot3;

	// ── 하단: 미장착 장비 목록 ──────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> ScrollUnequippedList;

	// ── 에디터 설정 ─────────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UUserWidget> PawnListItemClass;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UUserWidget> EquipListItemClass;

private:
	static constexpr int32 EquipSlotCount = 4;

	// 슬롯 인덱스 → 장비 슬롯 타입 매핑
	static const EEquipSlotType SlotTypeMap[EquipSlotCount];

	// 서브시스템 캐시
	TWeakObjectPtr<UPawnSubsystem> PawnSubRef;
	TWeakObjectPtr<UItemSubsystem> ItemSubRef;

	// 슬롯 배열
	TArray<UButton*> EquipSlotButtons;
	TArray<UTextBlock*> EquipSlotTexts;

	// 선택 상태
	TWeakObjectPtr<UDPawn> SelectedPawn;
	TWeakObjectPtr<UDEquipment> SelectedEquip;

	// ── 핸들러 ──────────────────────────────────────
	UFUNCTION() void HandleCloseClicked();

	UFUNCTION() void HandleEquipSlot0Clicked();
	UFUNCTION() void HandleEquipSlot1Clicked();
	UFUNCTION() void HandleEquipSlot2Clicked();
	UFUNCTION() void HandleEquipSlot3Clicked();
	void HandleEquipSlotClicked(int32 SlotIndex);

	UFUNCTION() void HandlePawnSelected(UDPawn* InPawn);
	UFUNCTION() void HandleEquipItemSelected(UDEquipment* InEquip);
	UFUNCTION() void HandleStatsChanged();

	// ── 갱신 ────────────────────────────────────────
	void RefreshPawnList();
	void RefreshPawnStats();
	void RefreshEquipSlots();
	void RefreshUnequippedList();

	// 선택된 폰에서 특정 슬롯 타입의 장비 찾기
	UDEquipment* FindEquipInSlot(int32 SlotIndex) const;
};
