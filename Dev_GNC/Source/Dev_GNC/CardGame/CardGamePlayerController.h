// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "CardGamePlayerController.generated.h"

/**
 * ACardGamePlayerController
 * 카드 게임 전용 플레이어 컨트롤러.
 * 마우스 커서 표시 및 UI+Game 입력 모드 설정.
 */
UCLASS()
class ACardGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ACardGamePlayerController();

protected:
	virtual void BeginPlay() override;
};
