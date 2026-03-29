// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PawnSubsystem.generated.h"

class UDPawn;

/**
 * UPawnSubsystem
 * 플레이어가 보유한 폰(용병) 목록을 관리하는 게임인스턴스 서브시스템.
 * Unity PawnManager (MonoSingleton) 대응.
 */
UCLASS()
class UPawnSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "CardGame|Pawn")
	void AddPawn(UDPawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Pawn")
	void RemovePawn(UDPawn* Pawn);

	UFUNCTION(BlueprintPure, Category = "CardGame|Pawn")
	int32 GetPawnCount() const { return PawnList.Num(); }

	const TArray<TObjectPtr<UDPawn>>& GetPawns() const { return PawnList; }

	UFUNCTION(BlueprintCallable, Category = "CardGame|Pawn")
	void ResetAll();

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Pawn")
	TArray<TObjectPtr<UDPawn>> PawnList;
};
