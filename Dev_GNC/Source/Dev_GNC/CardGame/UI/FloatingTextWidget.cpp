// Copyright GNC Project. All Rights Reserved.

#include "FloatingTextWidget.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogFloatingText, Log, All);

void UFloatingTextWidget::Show(EFloatingTextType Type, const FString& Text, FVector WorldPos)
{
	TrackedWorldPos = WorldPos;
	RemainingTime = Duration;
	SetVisibility(ESlateVisibility::HitTestInvisible);
	BP_PlayAnimation(Type, Text);
}

void UFloatingTextWidget::Hide()
{
	RemainingTime = 0.f;
	SetVisibility(ESlateVisibility::Collapsed);
}

void UFloatingTextWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (RemainingTime <= 0.f) return;

	RemainingTime -= InDeltaTime;

	if (RemainingTime <= 0.f)
	{
		Hide();
		return;
	}

	// 월드 좌표 → 스크린 좌표 프로젝션
	APlayerController* PC = GetOwningPlayer();
	if (!PC) return;

	FVector2D ScreenPos;
	if (UGameplayStatics::ProjectWorldToScreen(PC, TrackedWorldPos, ScreenPos))
	{
		// 뷰포트 스케일 보정
		const float Scale = UWidgetLayoutLibrary::GetViewportScale(this);
		if (Scale > 0.f)
		{
			SetRenderTranslation(ScreenPos / Scale);
		}
	}
}
