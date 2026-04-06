// Copyright GNC Project. All Rights Reserved.

#include "CardGamePlayerController.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Subsystem/StageSubsystem.h"
#include "Grid/SquareTile.h"
#include "Grid/CardGameActor.h"
#include "Grid/GridSlotComponent.h"
#include "Data/DObject.h"

DEFINE_LOG_CATEGORY_STATIC(LogCardGameInput, Log, All);

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

void ACardGamePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ClickAction)
		{
			EIC->BindAction(ClickAction, ETriggerEvent::Triggered, this, &ACardGamePlayerController::HandleClick);
		}
	}
}

void ACardGamePlayerController::HandleClick(const FInputActionValue& Value)
{
	UStageSubsystem* Stage = GetWorld()->GetSubsystem<UStageSubsystem>();
	if (!Stage || Stage->bIsBusy) return;

	FHitResult Hit;
	if (GetHitResultUnderCursorByChannel(
			UEngineTypes::ConvertToTraceType(ECC_Visibility), true, Hit))
	{
		if (ASquareTile* Tile = Cast<ASquareTile>(Hit.GetActor()))
		{
			ACardGameActor* Actor = Tile->Slot ? Cast<ACardGameActor>(Tile->Slot->SlotEntity) : nullptr;
			UE_LOG(LogCardGameInput, Log, TEXT("Tile clicked: ID=%d, Pos=(%d,%d), Actor=%s"),
				Tile->TileId, Tile->GridPosition.X, Tile->GridPosition.Y,
				Actor && Actor->Data ? *Actor->Data->GetName() : TEXT("None"));
			Stage->SelectTile(Tile);
		}
	}
}
