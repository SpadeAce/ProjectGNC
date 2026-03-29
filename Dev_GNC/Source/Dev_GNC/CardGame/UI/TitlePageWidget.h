// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PageWidget.h"
#include "TitlePageWidget.generated.h"

class UButton;

/**
 * UTitlePageWidget
 * 타이틀 화면 페이지. 뉴게임/로드/옵션/종료 버튼.
 * Unity TitlePage (PageView) 대응.
 */
UCLASS(Abstract, BlueprintType)
class UTitlePageWidget : public UPageWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativePreClose() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonNewGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonLoadGame;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonOption;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonExit;

private:
	UFUNCTION() void HandleNewGameClicked();
	UFUNCTION() void HandleLoadGameClicked();
	UFUNCTION() void HandleOptionClicked();
	UFUNCTION() void HandleExitClicked();

	void ResetAllSubsystems();
};
