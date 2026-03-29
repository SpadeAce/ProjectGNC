// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "CardGameMode.generated.h"

class UPageWidget;

/**
 * ACardGameMode
 * 카드 게임 공통 GameMode 베이스.
 * Title/Lobby/Stage 맵에서 사용하며, BP에서 InitialPageClass를 씬별로 설정한다.
 * Unity SceneBase 대응.
 */
UCLASS()
class ACardGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ACardGameMode();

	/** 맵 진입 시 자동으로 여는 초기 페이지 BP 클래스. 맵별 BP에서 설정. */
	UPROPERTY(EditAnywhere, Category = "CardGame|UI")
	TSubclassOf<UPageWidget> InitialPageClass;
};
