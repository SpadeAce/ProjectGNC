// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CardGameCameraPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * ACardGameCameraPawn
 * 탑다운 전략 카메라 폰. 우클릭 드래그 팬, Q/E 회전, 스크롤 줌.
 * Unity CameraController.cs 대응.
 */
UCLASS()
class ACardGameCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACardGameCameraPawn();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	/** 카메라 중심점을 강제 설정한다. StageGameMode에서 호출. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Camera")
	void SetPivot(FVector WorldPos);

protected:
	virtual void BeginPlay() override;

	// ── 컴포넌트 ────────────────────────────────
	UPROPERTY(VisibleAnywhere, Category = "CardGame|Camera")
	TObjectPtr<USceneComponent> PivotComponent;

	UPROPERTY(VisibleAnywhere, Category = "CardGame|Camera")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "CardGame|Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	// ── Enhanced Input ──────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Input")
	TObjectPtr<UInputMappingContext> CameraMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Input")
	TObjectPtr<UInputAction> PanAction;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Input")
	TObjectPtr<UInputAction> RotateAction;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Input")
	TObjectPtr<UInputAction> ZoomAction;

	// ── 카메라 설정 ─────────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float PanSpeed = 20.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float RotationInterpSpeed = 5.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float RotationStep = 90.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float MinZoomDistance = 300.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float MaxZoomDistance = 3000.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float ZoomSpeed = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float DefaultZoomDistance = 1200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float CameraElevation = 45.0f;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Camera")
	float InitialYaw = 45.0f;

private:
	// ── 입력 핸들러 ─────────────────────────────
	void HandlePan(const FInputActionValue& Value);
	void HandleRotate(const FInputActionValue& Value);
	void HandleZoom(const FInputActionValue& Value);

	// ── 내부 상태 ───────────────────────────────
	float TargetYaw = 0.0f;
	float CurrentYaw = 0.0f;
	float TargetArmLength = 1200.0f;
};
