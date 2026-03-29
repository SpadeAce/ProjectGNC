// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HUDActorWidget.generated.h"

class UDPawn;
class UDMonster;
class UProgressBar;
class UTextBlock;

/**
 * UHUDActorWidget
 * 캐릭터 머리 위에 표시되는 HP바/방어/이름 위젯.
 * Unity HUD_Actor.cs 대응.
 * UWidgetComponent(Screen Space)에 부착되어 자동 빌보드 처리.
 */
UCLASS(Abstract, BlueprintType)
class UHUDActorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 폰 데이터로 초기화. OnStatsChanged 구독. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void InitFromPawn(UDPawn* Pawn);

	/** 몬스터 데이터로 초기화. OnStatsChanged 구독. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void InitFromMonster(UDMonster* Monster);

protected:
	// ── BindWidget (BP에서 동일 이름 위젯 필요) ────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BarHP;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> BarShield;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextArmor;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> TextAmmo;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextName;

	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void Refresh();

	TWeakObjectPtr<UDPawn> BoundPawn;
	TWeakObjectPtr<UDMonster> BoundMonster;
	int32 MaxHP = 1;
};
