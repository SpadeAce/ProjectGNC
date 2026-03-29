// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CardGameUITypes.generated.h"

/**
 * FViewParam
 * 뷰(페이지/팝업)를 열 때 데이터를 전달하는 경량 구조체.
 * Unity ViewParam (Dictionary) 대응.
 */
USTRUCT(BlueprintType)
struct FViewParam
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "CardGame|UI")
	TObjectPtr<UObject> Payload = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "CardGame|UI")
	bool bFlag = false;

	UPROPERTY(BlueprintReadWrite, Category = "CardGame|UI")
	int32 IntValue = 0;
};
