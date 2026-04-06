// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CardGamePlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

/**
 * ACardGamePlayerController
 * 카드 게임 전용 플레이어 컨트롤러.
 * 마우스 커서 표시, UI+Game 입력 모드 설정, 타일 클릭 처리.
 */
UCLASS()
class ACardGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACardGamePlayerController();

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// ── Enhanced Input ──────────────────────────
	UPROPERTY(EditDefaultsOnly, Category = "CardGame|Input")
	TObjectPtr<UInputAction> ClickAction;

private:
	void HandleClick(const FInputActionValue& Value);
};
