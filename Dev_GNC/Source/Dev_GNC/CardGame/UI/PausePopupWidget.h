// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PopupWidget.h"
#include "PausePopupWidget.generated.h"

class UButton;

/**
 * UPausePopupWidget
 * 전투 중 일시정지 팝업. 계속/옵션/나가기 버튼.
 * Unity PausePopup.cs 대응.
 */
UCLASS(Abstract, BlueprintType)
class UPausePopupWidget : public UPopupWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativePreClose() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonContinue;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonOption;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonExit;

private:
	UFUNCTION() void HandleContinueClicked();
	UFUNCTION() void HandleOptionClicked();
	UFUNCTION() void HandleExitClicked();
};
