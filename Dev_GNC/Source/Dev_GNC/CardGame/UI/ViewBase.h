// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardGameUITypes.h"
#include "ViewBase.generated.h"

class ACardGameHUD;

/**
 * UViewBase
 * 모든 페이지/팝업의 추상 베이스 위젯.
 * Unity ViewCore (abstract) 대응.
 *
 * 라이프사이클: NativePreOpen → NativeOnOpened → ... → NativePreClose → NativeOnClosed
 */
UCLASS(Abstract, BlueprintType)
class UViewBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// ── 라이프사이클 (C++ 서브클래스 오버라이드) ──────

	/** 뷰 열기 전 호출. Param 저장 + 초기화. */
	virtual void NativePreOpen(const FViewParam& Param);

	/** 뷰가 열린 직후 호출. BP 훅 호출. */
	virtual void NativeOnOpened();

	/** 뷰 닫기 전 호출. 정리 로직. */
	virtual void NativePreClose();

	/** 뷰가 닫힌 직후 호출. BP 훅 호출. */
	virtual void NativeOnClosed();

	// ── Blueprint 훅 ─────────────────────────────────

	UFUNCTION(BlueprintImplementableEvent, Category = "CardGame|UI")
	void BP_OnOpened();

	UFUNCTION(BlueprintImplementableEvent, Category = "CardGame|UI")
	void BP_OnClosed();

	// ── 닫기 요청 ────────────────────────────────────

	/** HUD에 닫기를 위임한다. 페이지면 ClosePage, 팝업이면 ClosePopup. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void RequestClose();

	// ── Getter ───────────────────────────────────────

	UFUNCTION(BlueprintPure, Category = "CardGame|UI")
	const FViewParam& GetViewParam() const { return ViewParam; }

	void SetOwningHUD(ACardGameHUD* HUD);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "CardGame|UI")
	FViewParam ViewParam;

	TWeakObjectPtr<ACardGameHUD> OwningHUD;
};
