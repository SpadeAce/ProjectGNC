// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CardGameMode.h"
#include "CardStageGameMode.generated.h"

/**
 * ACardStageGameMode
 * 전투 스테이지 전용 GameMode.
 * BeginPlay에서 난이도별 프리셋을 로드하고 StageSubsystem::LoadStage()를 호출한다.
 * Unity StageScript.OnEnterScene() 대응.
 */
UCLASS()
class ACardStageGameMode : public ACardGameMode
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

private:
	void LoadStageFromPreset();
	void SetCameraPivot();
};
