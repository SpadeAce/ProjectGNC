// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PageWidget.h"
#include "LobbyPageWidget.generated.h"

class UButton;
class UTextBlock;
class UDeckSubsystem;
class UPawnSubsystem;
class ULobbySubsystem;
class UPlayerDataSubsystem;

/**
 * ULobbyPageWidget
 * 로비 메인 페이지. 전투 시작, 덱 편성, 폰 관리, 상점, 모집, 종료 버튼.
 * Unity LobbyPage (PageView) 대응.
 */
UCLASS(Abstract, BlueprintType)
class ULobbyPageWidget : public UPageWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativeOnOpened() override;
	virtual void NativePreClose() override;

protected:
	// ── BindWidget ──────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonStartBattle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonDeckSetting;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonPawnManage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonShop;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonRecruit;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonExit;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextGold;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextPawnCount;

	// ── 서브 페이지 클래스 ─────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UPageWidget> DeckSettingPageClass;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UPageWidget> PawnManagePageClass;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UPageWidget> ShopPageClass;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UPageWidget> RecruitPageClass;

private:
	// ── 서브시스템 캐시 ─────────────────────────────
	TWeakObjectPtr<UDeckSubsystem> DeckSubRef;
	TWeakObjectPtr<UPawnSubsystem> PawnSubRef;
	TWeakObjectPtr<ULobbySubsystem> LobbySubRef;
	TWeakObjectPtr<UPlayerDataSubsystem> PlayerSubRef;

	// ── 핸들러 ──────────────────────────────────────
	UFUNCTION() void HandleStartBattleClicked();
	UFUNCTION() void HandleDeckSettingClicked();
	UFUNCTION() void HandlePawnManageClicked();
	UFUNCTION() void HandleShopClicked();
	UFUNCTION() void HandleRecruitClicked();
	UFUNCTION() void HandleExitClicked();

	// ── 내부 헬퍼 ───────────────────────────────────
	void RefreshInfoDisplay();
	void EnsureTestData();
};
