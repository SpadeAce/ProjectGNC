// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PlayerDataSubsystem.generated.h"

/**
 * UPlayerDataSubsystem
 * 플레이어 메타 데이터(골드, 난이도)를 관리하는 게임인스턴스 서브시스템.
 * Unity PlayerManager (MonoSingleton) 대응.
 */
UCLASS()
class UPlayerDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 골드 ─────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "CardGame|Player")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Player")
	bool SpendGold(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "CardGame|Player")
	bool HasEnoughGold(int32 Amount) const;

	// ── 난이도 ───────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "CardGame|Player")
	void IncrementDifficulty();

	// ── 리셋 ─────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "CardGame|Player")
	void ResetAll();

	// ── 상태 ─────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Player")
	int32 Gold = 1000;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Player")
	int32 DifficultyLevel = 1;
};
