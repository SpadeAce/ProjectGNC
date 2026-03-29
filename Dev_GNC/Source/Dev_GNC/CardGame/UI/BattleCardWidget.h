// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BattleCardWidget.generated.h"

class UDCard;
class UDPawn;
class ACardGameActor;
class UTextBlock;
class UImage;

/**
 * UBattleCardWidget
 * 핸드 카드 1장을 표시하고 드래그를 시작하는 위젯.
 * Unity BattleCard.cs 대응.
 */
UCLASS(Abstract, BlueprintType)
class UBattleCardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 카드 데이터 설정. Owner가 nullptr이면 글로벌 핸드 카드. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void SetData(UDCard* InCard, UDPawn* InOwner = nullptr);

	/** 비활성 오버레이 표시/숨김 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void SetDisabled(bool bDisabled);

	/** 전체 위젯 표시/숨김 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|UI")
	void SetCardVisible(bool bVisible);

	/** 현재 카드 데이터 */
	UFUNCTION(BlueprintPure, Category = "CardGame|UI")
	UDCard* GetCard() const { return Card; }

	/** 카드 소유 폰 (글로벌이면 nullptr) */
	UFUNCTION(BlueprintPure, Category = "CardGame|UI")
	UDPawn* GetCardOwner() const { return CardOwner; }

	/** 드래그 시 사용할 캐스터 액터 (StagePageWidget이 설정) */
	void SetCasterActor(ACardGameActor* InActor);

protected:
	// ── BindWidget ──────────────────────────────────
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextEnergyCost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextAmmoCost;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TextDesc;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageIcon;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> ImageDimmed;

	// ── 드래그 ──────────────────────────────────────
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
	UPROPERTY()
	TObjectPtr<UDCard> Card;

	UPROPERTY()
	TObjectPtr<UDPawn> CardOwner;

	TObjectPtr<ACardGameActor> CasterActor;
};
