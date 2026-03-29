// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CardGameTypes.h"
#include "TurnSubsystem.generated.h"

class ACardGameActor;
class ASquareTile;

// ── 델리게이트 ──────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTurnChanged, int32, NewTurn);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnActingPowerChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMonsterTurnFinished);

/**
 * UTurnSubsystem
 * 턴 페이즈, Acting Power, 이동 추적, 몬스터 AI를 관리하는 월드 서브시스템.
 * Unity TurnManager (MonoSingleton) 대응.
 */
UCLASS()
class UTurnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 턴 제어 ────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "CardGame|Turn")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "CardGame|Turn")
	void StartPawnTurn(bool bDrawCards = true);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Turn")
	void EndPawnTurn();

	// ── Acting Power ───────────────────────────────
	static constexpr int32 SharedActingPowerPerTurn = 5;

	UFUNCTION(BlueprintCallable, Category = "CardGame|Turn")
	void ConsumeSharedActingPower(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Turn")
	void AddSharedActingPower(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "CardGame|Turn")
	bool HasEnoughSharedActingPower(int32 Cost) const;

	// ── 이동 추적 ──────────────────────────────────
	void OnActorMoved(ACardGameActor* Actor, int32 Remaining);
	void AdjustRemainingMovement(ACardGameActor* Actor, int32 Delta);

	UFUNCTION(BlueprintPure, Category = "CardGame|Turn")
	int32 GetRemainingMovement(ACardGameActor* Actor) const;

	UFUNCTION(BlueprintPure, Category = "CardGame|Turn")
	bool HasActorMoved(ACardGameActor* Actor) const;

	// ── 쿼리 ───────────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "CardGame|Turn")
	bool IsPawnTurn() const { return CurrentPhase == ETurnPhase::Pawn; }

	UFUNCTION(BlueprintPure, Category = "CardGame|Turn")
	bool IsMonsterTurn() const { return CurrentPhase == ETurnPhase::Monster; }

	UFUNCTION(BlueprintCallable, Category = "CardGame|Turn")
	void SetTurnLimit(int32 Limit);

	// ── 상태 ───────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Turn")
	ETurnPhase CurrentPhase = ETurnPhase::Pawn;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Turn")
	int32 CurrentTurn = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Turn")
	int32 TurnLimit = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Turn")
	int32 SharedActingPower = 0;

	// ── 델리게이트 ─────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "CardGame|Turn")
	FOnTurnChanged OnTurnChanged;

	UPROPERTY(BlueprintAssignable, Category = "CardGame|Turn")
	FOnActingPowerChanged OnActingPowerChanged;

	UPROPERTY(BlueprintAssignable, Category = "CardGame|Turn")
	FOnMonsterTurnFinished OnMonsterTurnFinished;

private:
	// ── 이동 추적 ──────────────────────────────────
	TMap<TWeakObjectPtr<ACardGameActor>, int32> RemainingMovement;

	// ── 몬스터 턴 처리 ─────────────────────────────
	TArray<TWeakObjectPtr<ACardGameActor>> PendingMonsters;
	TArray<TWeakObjectPtr<ACardGameActor>> CachedUsers;
	int32 CurrentMonsterIndex = 0;
	bool bIsAlert = false;

	void StartMonsterTurn();
	void ProcessNextMonster();
	void MonsterAct(ACardGameActor* Monster);
	void OnMonsterActionComplete();

	// ── 공격 ───────────────────────────────────────
	void ExecuteAttack(ACardGameActor* Attacker, ACardGameActor* Target, int32 AttackPower, TFunction<void()> OnComplete);

	// ── AI 헬퍼 ────────────────────────────────────
	bool CheckAlertState();
	ACardGameActor* GetNearestActor(ASquareTile* From, const TArray<TWeakObjectPtr<ACardGameActor>>& Targets);
	ASquareTile* GetBestTileToward(ASquareTile* From, ASquareTile* TargetTile, int32 Movement);
	ASquareTile* GetTileForActor(ACardGameActor* Actor) const;

	FTimerHandle MonsterTimerHandle;
};
