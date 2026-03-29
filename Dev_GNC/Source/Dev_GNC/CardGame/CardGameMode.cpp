// Copyright GNC Project. All Rights Reserved.

#include "CardGameMode.h"
#include "CardGamePlayerController.h"
#include "CardGameCameraPawn.h"
#include "UI/CardGameHUD.h"

ACardGameMode::ACardGameMode()
{
	HUDClass = ACardGameHUD::StaticClass();
	PlayerControllerClass = ACardGamePlayerController::StaticClass();
	DefaultPawnClass = ACardGameCameraPawn::StaticClass();
}
