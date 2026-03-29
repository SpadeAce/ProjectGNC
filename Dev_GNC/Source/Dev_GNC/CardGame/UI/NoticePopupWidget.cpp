// Copyright GNC Project. All Rights Reserved.

#include "NoticePopupWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UNoticePopupWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	if (ButtonConfirm) ButtonConfirm->OnClicked.AddDynamic(this, &UNoticePopupWidget::HandleConfirmClicked);
	if (ButtonCancel)  ButtonCancel->OnClicked.AddDynamic(this, &UNoticePopupWidget::HandleCancelClicked);
}

void UNoticePopupWidget::NativePreClose()
{
	if (ButtonConfirm) ButtonConfirm->OnClicked.RemoveAll(this);
	if (ButtonCancel)  ButtonCancel->OnClicked.RemoveAll(this);

	OnConfirm.Unbind();
	OnCancel.Unbind();

	Super::NativePreClose();
}

void UNoticePopupWidget::SetContent(const FText& Title, const FText& Body)
{
	if (TextTitle) TextTitle->SetText(Title);
	if (TextBody)  TextBody->SetText(Body);
}

void UNoticePopupWidget::HandleConfirmClicked()
{
	OnConfirm.ExecuteIfBound();
	RequestClose();
}

void UNoticePopupWidget::HandleCancelClicked()
{
	OnCancel.ExecuteIfBound();
	RequestClose();
}
