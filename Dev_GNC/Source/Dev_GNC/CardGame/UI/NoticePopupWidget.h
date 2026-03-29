// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PopupWidget.h"
#include "NoticePopupWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_DELEGATE(FOnNoticeConfirm);
DECLARE_DELEGATE(FOnNoticeCancel);

/**
 * UNoticePopupWidget
 * 범용 확인/취소 다이얼로그 팝업.
 * Unity NoticePopup.cs 대응.
 *
 * 사용법:
 *   auto* Popup = HUD->OpenPopup<UNoticePopupWidget>();
 *   Popup->SetContent(Title, Body);
 *   Popup->OnConfirm.BindUObject(this, &ThisClass::HandleConfirm);
 */
UCLASS(Abstract, BlueprintType)
class UNoticePopupWidget : public UPopupWidget
{
	GENERATED_BODY()

public:
	virtual void NativePreOpen(const FViewParam& Param) override;
	virtual void NativePreClose() override;

	/** 제목과 본문 텍스트를 설정한다. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void SetContent(const FText& Title, const FText& Body);

	/** 확인 버튼 콜백. 팝업 열기 후 외부에서 바인딩. */
	FOnNoticeConfirm OnConfirm;

	/** 취소 버튼 콜백. 팝업 열기 후 외부에서 바인딩. */
	FOnNoticeCancel OnCancel;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextTitle;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextBody;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonConfirm;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ButtonCancel;

private:
	UFUNCTION() void HandleConfirmClicked();
	UFUNCTION() void HandleCancelClicked();
};
