// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CardGameTypes.h"
#include "FloatingTextWidget.generated.h"

/**
 * UFloatingTextWidget
 * 전투 중 데미지/힐/버프 등을 표시하는 플로팅 텍스트.
 * ACardGameHUD에서 풀링(10개)하여 라운드 로빈으로 재사용한다.
 * Unity FloatingText 대응.
 *
 * BP에서 BP_PlayAnimation을 구현하여 타입별 연출(색상, 이동, 페이드)을 처리한다.
 */
UCLASS(Abstract, BlueprintType)
class UFloatingTextWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 텍스트를 표시하고 애니메이션을 시작한다. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void Show(EFloatingTextType Type, const FString& Text, FVector WorldPos);

	/** 즉시 숨긴다. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void Hide();

	/** BP에서 타입별 애니메이션을 재생한다. */
	UFUNCTION(BlueprintImplementableEvent, Category = "CardGame|UI")
	void BP_PlayAnimation(EFloatingTextType Type, const FString& Text);

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	FVector TrackedWorldPos = FVector::ZeroVector;
	float RemainingTime = 0.f;

	static constexpr float Duration = 1.0f;
};
