// Copyright GNC Project. All Rights Reserved.

#include "CardGameCameraPawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputActionValue.h"

ACardGameCameraPawn::ACardGameCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 루트: 피벗 (팬 이동의 기준점)
	PivotComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Pivot"));
	RootComponent = PivotComponent;

	// 스프링 암: 거리 + 앙각
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(PivotComponent);
	SpringArmComponent->bDoCollisionTest = false;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 8.0f;
	SpringArmComponent->TargetArmLength = DefaultZoomDistance;
	SpringArmComponent->SetRelativeRotation(FRotator(-CameraElevation, 0.0f, 0.0f));

	// 카메라
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent);
}

void ACardGameCameraPawn::BeginPlay()
{
	Super::BeginPlay();

	TargetArmLength = DefaultZoomDistance;
	TargetYaw = InitialYaw;
	CurrentYaw = InitialYaw;

	// Enhanced Input Mapping Context 등록
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
				ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (CameraMappingContext)
			{
				Subsystem->AddMappingContext(CameraMappingContext, 0);
			}
		}
	}
}

void ACardGameCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Yaw 보간
	CurrentYaw = FMath::FInterpTo(CurrentYaw, TargetYaw, DeltaTime, RotationInterpSpeed);
	FRotator NewRot = GetActorRotation();
	NewRot.Yaw = CurrentYaw;
	SetActorRotation(NewRot);

	// 줌 보간
	SpringArmComponent->TargetArmLength = FMath::FInterpTo(
		SpringArmComponent->TargetArmLength, TargetArmLength, DeltaTime, 8.0f);
}

void ACardGameCameraPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (PanAction)    EIC->BindAction(PanAction,    ETriggerEvent::Triggered, this, &ACardGameCameraPawn::HandlePan);
		if (RotateAction) EIC->BindAction(RotateAction, ETriggerEvent::Triggered, this, &ACardGameCameraPawn::HandleRotate);
		if (ZoomAction)   EIC->BindAction(ZoomAction,   ETriggerEvent::Triggered, this, &ACardGameCameraPawn::HandleZoom);
	}
}

void ACardGameCameraPawn::SetPivot(FVector WorldPos)
{
	SetActorLocation(WorldPos);
}

// ────────────────────────────────────────────────────────────
// 입력 핸들러
// ────────────────────────────────────────────────────────────

void ACardGameCameraPawn::HandlePan(const FInputActionValue& Value)
{
	const FVector2D Input = Value.Get<FVector2D>();
	if (Input.IsNearlyZero()) return;

	// 현재 Yaw 기준으로 Forward/Right 벡터 계산
	const FRotator YawRot(0.0f, CurrentYaw, 0.0f);
	const FVector Forward = YawRot.RotateVector(FVector::ForwardVector);
	const FVector Right = YawRot.RotateVector(FVector::RightVector);

	const float ZoomFactor = SpringArmComponent->TargetArmLength / DefaultZoomDistance;
	const FVector Delta = -(Forward * Input.Y + Right * Input.X) * PanSpeed * ZoomFactor;
	AddActorWorldOffset(Delta);
}

void ACardGameCameraPawn::HandleRotate(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	if (FMath::IsNearlyZero(Input)) return;

	// Q=-1, E=+1 → RotationStep 만큼 회전
	TargetYaw += FMath::Sign(Input) * RotationStep;
}

void ACardGameCameraPawn::HandleZoom(const FInputActionValue& Value)
{
	const float Input = Value.Get<float>();
	if (FMath::IsNearlyZero(Input)) return;

	TargetArmLength = FMath::Clamp(
		TargetArmLength - Input * ZoomSpeed,
		MinZoomDistance,
		MaxZoomDistance);
}
