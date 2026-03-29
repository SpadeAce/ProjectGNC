// Copyright GNC Project. All Rights Reserved.

#include "ViewBase.h"
#include "CardGameHUD.h"

DEFINE_LOG_CATEGORY_STATIC(LogViewBase, Log, All);

void UViewBase::NativePreOpen(const FViewParam& Param)
{
	ViewParam = Param;
}

void UViewBase::NativeOnOpened()
{
	BP_OnOpened();
}

void UViewBase::NativePreClose()
{
}

void UViewBase::NativeOnClosed()
{
	BP_OnClosed();
}

void UViewBase::SetOwningHUD(ACardGameHUD* HUD)
{
	OwningHUD = HUD;
}

void UViewBase::RequestClose()
{
	if (ACardGameHUD* HUD = OwningHUD.Get())
	{
		HUD->RequestCloseView(this);
	}
	else
	{
		UE_LOG(LogViewBase, Warning, TEXT("RequestClose: OwningHUD is null"));
	}
}
