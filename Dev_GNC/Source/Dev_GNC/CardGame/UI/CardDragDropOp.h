// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "CardDragDropOp.generated.h"

class UDCard;
class UDPawn;
class ACardGameActor;

/**
 * UCardDragDropOp
 * 카드 드래그 시 payload를 운반하는 드래그 오퍼레이션.
 * Unity BattleCard 드래그 이벤트 대응.
 */
UCLASS()
class UCardDragDropOp : public UDragDropOperation
{
	GENERATED_BODY()

public:
	/** 드래그 중인 카드 */
	UPROPERTY(BlueprintReadWrite, Category = "CardGame|UI")
	TObjectPtr<UDCard> Card;

	/** 카드 소유자. nullptr이면 글로벌 핸드, non-null이면 폰 핸드 카드. */
	UPROPERTY(BlueprintReadWrite, Category = "CardGame|UI")
	TObjectPtr<UDPawn> CardOwner;

	/** 카드를 시전할 액터 */
	UPROPERTY(BlueprintReadWrite, Category = "CardGame|UI")
	TObjectPtr<ACardGameActor> CasterActor;
};
