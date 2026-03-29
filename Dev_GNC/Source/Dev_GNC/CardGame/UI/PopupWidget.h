// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ViewBase.h"
#include "PopupWidget.generated.h"

/**
 * UPopupWidget
 * 모달/다이얼로그 팝업 위젯의 추상 베이스.
 * 다중 공존 가능. ACardGameHUD가 관리한다.
 * Unity PopupView 대응.
 */
UCLASS(Abstract, BlueprintType)
class UPopupWidget : public UViewBase
{
	GENERATED_BODY()

public:
	/** 팝업 간 Z순서. 높을수록 위에 표시. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "CardGame|UI")
	int32 PopupZOrder = 0;
};
