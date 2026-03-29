// Copyright GNC Project. All Rights Reserved.

#include "TitlePageWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "PlayerDataSubsystem.h"
#include "PawnSubsystem.h"
#include "DeckSubsystem.h"
#include "ItemSubsystem.h"
#include "LobbySubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogTitlePage, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void UTitlePageWidget::NativePreOpen(const FViewParam& Param)
{
	Super::NativePreOpen(Param);

	if (ButtonNewGame)  ButtonNewGame->OnClicked.AddDynamic(this, &UTitlePageWidget::HandleNewGameClicked);
	if (ButtonLoadGame) ButtonLoadGame->OnClicked.AddDynamic(this, &UTitlePageWidget::HandleLoadGameClicked);
	if (ButtonOption)   ButtonOption->OnClicked.AddDynamic(this, &UTitlePageWidget::HandleOptionClicked);
	if (ButtonExit)     ButtonExit->OnClicked.AddDynamic(this, &UTitlePageWidget::HandleExitClicked);
}

void UTitlePageWidget::NativePreClose()
{
	if (ButtonNewGame)  ButtonNewGame->OnClicked.RemoveAll(this);
	if (ButtonLoadGame) ButtonLoadGame->OnClicked.RemoveAll(this);
	if (ButtonOption)   ButtonOption->OnClicked.RemoveAll(this);
	if (ButtonExit)     ButtonExit->OnClicked.RemoveAll(this);

	Super::NativePreClose();
}

// ────────────────────────────────────────────────────────────
// Handlers
// ────────────────────────────────────────────────────────────

void UTitlePageWidget::HandleNewGameClicked()
{
	ResetAllSubsystems();
	UGameplayStatics::OpenLevel(this, FName("Lobby"));
}

void UTitlePageWidget::HandleLoadGameClicked()
{
	// 세이브/로드 미구현. 현재는 NewGame과 동일 동작 (Unity 원본도 동일).
	UGameplayStatics::OpenLevel(this, FName("Lobby"));
}

void UTitlePageWidget::HandleOptionClicked()
{
	// Phase 5-E 스텁
	UE_LOG(LogTitlePage, Log, TEXT("Option clicked — not yet implemented (Phase 5-E)"));
}

void UTitlePageWidget::HandleExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}

// ────────────────────────────────────────────────────────────
// Private
// ────────────────────────────────────────────────────────────

void UTitlePageWidget::ResetAllSubsystems()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	if (auto* Sub = GI->GetSubsystem<UPlayerDataSubsystem>()) Sub->ResetAll();
	if (auto* Sub = GI->GetSubsystem<UPawnSubsystem>())       Sub->ResetAll();
	if (auto* Sub = GI->GetSubsystem<UDeckSubsystem>())       Sub->ResetAll();
	if (auto* Sub = GI->GetSubsystem<UItemSubsystem>())       Sub->ResetAll();
	if (auto* Sub = GI->GetSubsystem<ULobbySubsystem>())      Sub->ResetAll();

	UE_LOG(LogTitlePage, Log, TEXT("All subsystems reset for new game"));
}
