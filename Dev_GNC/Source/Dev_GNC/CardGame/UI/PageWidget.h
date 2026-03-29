// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ViewBase.h"
#include "PageWidget.generated.h"

/**
 * UPageWidget
 * 전체 화면 페이지 위젯의 추상 베이스.
 * ACardGameHUD가 스택으로 관리한다.
 * Unity PageView 대응.
 */
UCLASS(Abstract, BlueprintType)
class UPageWidget : public UViewBase
{
	GENERATED_BODY()

public:
	/** true이면 다른 페이지가 위에 열려도 이 페이지를 숨기지 않는다. Unity _dontHide 대응. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CardGame|UI")
	bool bKeepPreviousVisible = false;
};
