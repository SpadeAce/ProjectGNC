// Copyright GNC Project. All Rights Reserved.

#include "PausePopupWidget.h"
#include "CardGameHUD.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerDataSubsystem.h"
#include "PawnSubsystem.h"
#include "DeckSubsystem.h"
#include "ItemSubsystem.h"
#include "LobbySubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogPausePopup, Log, All);

void UPausePopupWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	if (ButtonContinue) ButtonContinue->OnClicked.AddDynamic(this, &UPausePopupWidget::HandleContinueClicked);
	if (ButtonOption)   ButtonOption->OnClicked.AddDynamic(this, &UPausePopupWidget::HandleOptionClicked);
	if (ButtonExit)     ButtonExit->OnClicked.AddDynamic(this, &UPausePopupWidget::HandleExitClicked);
}

void UPausePopupWidget::NativePreClose()
{
	if (ButtonContinue) ButtonContinue->OnClicked.RemoveAll(this);
	if (ButtonOption)   ButtonOption->OnClicked.RemoveAll(this);
	if (ButtonExit)     ButtonExit->OnClicked.RemoveAll(this);

	Super::NativePreClose();
}

void UPausePopupWidget::HandleContinueClicked()
{
	RequestClose();
}

void UPausePopupWidget::HandleOptionClicked()
{
	// OptionPopup은 Phase 5-E 범위 외 — 추후 구현
	UE_LOG(LogPausePopup, Log, TEXT("Option clicked — not yet implemented"));
}

void UPausePopupWidget::HandleExitClicked()
{
	// 서브시스템 리셋 후 타이틀로 복귀
	UGameInstance* GI = GetGameInstance();
	if (GI)
	{
		if (auto* Sub = GI->GetSubsystem<UPlayerDataSubsystem>()) Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UPawnSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UDeckSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<UItemSubsystem>())       Sub->ResetAll();
		if (auto* Sub = GI->GetSubsystem<ULobbySubsystem>())      Sub->ResetAll();
	}

	UE_LOG(LogPausePopup, Log, TEXT("Exit clicked — returning to title"));
	UGameplayStatics::OpenLevel(this, FName("Title"));
}
