// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CardGameUITypes.h"
#include "CardGameTypes.h"
#include "CardGameHUD.generated.h"

class UViewBase;
class UPageWidget;
class UPopupWidget;
class UFloatingTextWidget;
class UPanelWidget;

/**
 * ACardGameHUD
 * 카드 게임 UI 전체를 관리하는 HUD.
 * Unity UIManager + HudController 대응.
 *
 * - 페이지 스택: OpenPage/ClosePage (한 번에 하나의 페이지만 표시)
 * - 팝업 관리: OpenPopup/ClosePopup (다중 공존)
 * - 플로팅 텍스트 풀: 10개 사전 생성, 라운드 로빈
 */
UCLASS()
class ACardGameHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	// ── 페이지 스택 ─────────────────────────────────

	/** 페이지를 열고 스택에 Push. 이미 스택에 있으면 그 페이지 반환. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI", meta = (DeterminesOutputType = "PageClass"))
	UPageWidget* OpenPage(TSubclassOf<UPageWidget> PageClass, const FViewParam& Param = FViewParam());

	/** 스택 최상위 페이지를 닫는다. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void ClosePage();

	/** 모든 페이지를 닫는다. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void CloseAllPages();

	/** 현재 최상위 페이지 반환. 없으면 nullptr. */
	UFUNCTION(BlueprintPure, Category = "CardGame|UI")
	UPageWidget* GetTopPage() const;

	// ── 팝업 ────────────────────────────────────────

	/** 팝업을 열어 표시한다. 다중 팝업 공존 가능. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI", meta = (DeterminesOutputType = "PopupClass"))
	UPopupWidget* OpenPopup(TSubclassOf<UPopupWidget> PopupClass, const FViewParam& Param = FViewParam());

	/** 특정 팝업을 닫는다. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void ClosePopup(UPopupWidget* Popup);

	/** 모든 팝업을 닫는다. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void CloseAllPopups();

	// ── 플로팅 텍스트 ───────────────────────────────

	/** 월드 좌표에 플로팅 텍스트를 표시한다. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void ShowFloatingText(EFloatingTextType Type, const FString& Text, FVector WorldPos);

	// ── ViewBase에서 호출하는 닫기 디스패치 ─────────

	/** ViewBase::RequestClose()에서 호출. 페이지인지 팝업인지 판별하여 처리. */
	void RequestCloseView(UViewBase* View);

	// ── C++ 템플릿 유틸리티 ─────────────────────────

	template<typename T>
	T* OpenPage(const FViewParam& Param = FViewParam())
	{
		return Cast<T>(OpenPage(T::StaticClass(), Param));
	}

	template<typename T>
	T* OpenPopup(const FViewParam& Param = FViewParam())
	{
		return Cast<T>(OpenPopup(T::StaticClass(), Param));
	}

protected:
	/** 루트 위젯 BP 클래스 (CanvasPanel 2개: PageLayer, PopupLayer 포함) */
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UUserWidget> RootWidgetClass;

	/** 플로팅 텍스트 위젯 BP 클래스 */
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UFloatingTextWidget> FloatingTextClass;


private:
	// ── 내부 상태 ────────────────────────────────────

	UPROPERTY()
	TObjectPtr<UUserWidget> RootWidget;

	UPROPERTY()
	TObjectPtr<UPanelWidget> PageLayer;

	UPROPERTY()
	TObjectPtr<UPanelWidget> PopupLayer;

	TArray<TObjectPtr<UPageWidget>> PageStack;
	TArray<TObjectPtr<UPopupWidget>> ActivePopups;

	// 플로팅 텍스트 풀
	static constexpr int32 FloatingTextPoolSize = 10;
	UPROPERTY()
	TArray<TObjectPtr<UFloatingTextWidget>> FloatingTextPool;
	int32 FloatingTextNextIndex = 0;

	// ── 내부 헬퍼 ────────────────────────────────────

	void InitRootWidget();
	void InitFloatingTextPool();
};
