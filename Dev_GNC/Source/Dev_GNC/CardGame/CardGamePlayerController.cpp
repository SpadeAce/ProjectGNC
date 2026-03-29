// Copyright GNC Project. All Rights Reserved.

#include "CardGamePlayerController.h"

ACardGamePlayerController::ACardGamePlayerController()
{
	bShowMouseCursor = true;
}

void ACardGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	// UI 클릭과 게임 월드 입력 모두 허용
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}
