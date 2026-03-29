// Copyright GNC Project. All Rights Reserved.

#include "TurnSubsystem.h"
#include "StageSubsystem.h"
#include "GridSubsystem.h"
#include "DeckSubsystem.h"
#include "CardGameActor.h"
#include "SquareTile.h"
#include "GridSlotComponent.h"
#include "DPawn.h"
#include "DMonster.h"
#include "DObject.h"

DEFINE_LOG_CATEGORY_STATIC(LogTurnSubsystem, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void UTurnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogTurnSubsystem, Log, TEXT("UTurnSubsystem Initialized"));
}

void UTurnSubsystem::Deinitialize()
{
	GetWorld()->GetTimerManager().ClearTimer(MonsterTimerHandle);
	Super::Deinitialize();
}

// ────────────────────────────────────────────────────────────
// 턴 제어
// ────────────────────────────────────────────────────────────

void UTurnSubsystem::StartGame()
{
	CurrentTurn = 0;
	StartPawnTurn(/*bDrawCards=*/ false);
}

void UTurnSubsystem::StartPawnTurn(bool bDrawCards)
{
	CurrentTurn++;
	RemainingMovement.Reset();
	CurrentPhase = ETurnPhase::Pawn;
	SharedActingPower = SharedActingPowerPerTurn;

	UE_LOG(LogTurnSubsystem, Log, TEXT("=== Pawn Turn %d Start (AP: %d) ==="), CurrentTurn, SharedActingPower);

	// 모든 생존 액터의 버프 틱
	UStageSubsystem* StageSub = GetWorld()->GetSubsystem<UStageSubsystem>();
	if (StageSub)
	{
		for (auto& Pair : StageSub->UserActors)
		{
			if (ACardGameActor* Actor = Pair.Value)
			{
				if (UDPawn* Pawn = Cast<UDPawn>(Actor->Data))
				{
					if (!Pawn->IsDead())
					{
						Pawn->TickBuffs();
						if (bDrawCards)
						{
							Pawn->DrawCards(UDPawn::DrawPerTurn);
						}
					}
				}
			}
		}
		for (auto& Pair : StageSub->EnemyActors)
		{
			if (ACardGameActor* Actor = Pair.Value)
			{
				if (UDMonster* Monster = Cast<UDMonster>(Actor->Data))
				{
					if (!Monster->IsDead())
					{
						Monster->TickBuffs();
					}
				}
			}
		}
	}

	// 글로벌 덱 드로우 (턴당 1회)
	if (bDrawCards)
	{
		if (UDeckSubsystem* DeckSub = GetWorld()->GetGameInstance()->GetSubsystem<UDeckSubsystem>())
		{
			DeckSub->StartTurnDraw();
		}
	}

	OnTurnChanged.Broadcast(CurrentTurn);
	OnActingPowerChanged.Broadcast();
}

void UTurnSubsystem::EndPawnTurn()
{
	UE_LOG(LogTurnSubsystem, Log, TEXT("=== Pawn Turn %d End ==="), CurrentTurn);

	// 선택 해제
	UStageSubsystem* StageSub = GetWorld()->GetSubsystem<UStageSubsystem>();
	if (StageSub)
	{
		StageSub->Deselect();
	}

	CurrentPhase = ETurnPhase::Monster;
	StartMonsterTurn();
}

// ────────────────────────────────────────────────────────────
// Acting Power
// ────────────────────────────────────────────────────────────

void UTurnSubsystem::ConsumeSharedActingPower(int32 Amount)
{
	SharedActingPower = FMath::Max(0, SharedActingPower - Amount);
	OnActingPowerChanged.Broadcast();
}

void UTurnSubsystem::AddSharedActingPower(int32 Amount)
{
	SharedActingPower += Amount;
	OnActingPowerChanged.Broadcast();
}

bool UTurnSubsystem::HasEnoughSharedActingPower(int32 Cost) const
{
	return SharedActingPower >= Cost;
}

// ────────────────────────────────────────────────────────────
// 이동 추적
// ────────────────────────────────────────────────────────────

void UTurnSubsystem::OnActorMoved(ACardGameActor* Actor, int32 Remaining)
{
	if (!Actor) return;
	RemainingMovement.Add(TWeakObjectPtr<ACardGameActor>(Actor), Remaining);
}

void UTurnSubsystem::AdjustRemainingMovement(ACardGameActor* Actor, int32 Delta)
{
	if (!Actor) return;

	TWeakObjectPtr<ACardGameActor> Key(Actor);
	if (int32* Found = RemainingMovement.Find(Key))
	{
		*Found = FMath::Max(0, *Found + Delta);
	}
}

int32 UTurnSubsystem::GetRemainingMovement(ACardGameActor* Actor) const
{
	if (!Actor) return -1;

	TWeakObjectPtr<ACardGameActor> Key(const_cast<ACardGameActor*>(Actor));
	const int32* Found = RemainingMovement.Find(Key);
	return Found ? *Found : -1;
}

bool UTurnSubsystem::HasActorMoved(ACardGameActor* Actor) const
{
	const int32 Remaining = GetRemainingMovement(Actor);
	return Remaining != -1 && Remaining <= 0;
}

void UTurnSubsystem::SetTurnLimit(int32 Limit)
{
	TurnLimit = Limit;
}

// ────────────────────────────────────────────────────────────
// 몬스터 턴 (콜백 체인)
// ────────────────────────────────────────────────────────────

void UTurnSubsystem::StartMonsterTurn()
{
	UE_LOG(LogTurnSubsystem, Log, TEXT("=== Monster Turn Start ==="));

	UStageSubsystem* StageSub = GetWorld()->GetSubsystem<UStageSubsystem>();
	if (!StageSub) { StartPawnTurn(); return; }

	PendingMonsters.Reset();
	CachedUsers.Reset();

	// 생존 적 수집
	for (auto& Pair : StageSub->EnemyActors)
	{
		if (ACardGameActor* Actor = Pair.Value)
		{
			UDMonster* Mon = Cast<UDMonster>(Actor->Data);
			if (Mon && !Mon->IsDead())
			{
				PendingMonsters.Add(Actor);
			}
		}
	}

	// 생존 아군 수집
	for (auto& Pair : StageSub->UserActors)
	{
		if (ACardGameActor* Actor = Pair.Value)
		{
			UDPawn* Pawn = Cast<UDPawn>(Actor->Data);
			if (Pawn && !Pawn->IsDead())
			{
				CachedUsers.Add(Actor);
			}
		}
	}

	CurrentMonsterIndex = 0;
	bIsAlert = CheckAlertState();

	UE_LOG(LogTurnSubsystem, Log, TEXT("  Monsters: %d, Users: %d, Alert: %s"),
		PendingMonsters.Num(), CachedUsers.Num(), bIsAlert ? TEXT("YES") : TEXT("NO"));

	ProcessNextMonster();
}

void UTurnSubsystem::ProcessNextMonster()
{
	// 모든 몬스터 행동 완료
	if (CurrentMonsterIndex >= PendingMonsters.Num())
	{
		UE_LOG(LogTurnSubsystem, Log, TEXT("=== Monster Turn End ==="));

		// 턴 리밋 체크
		if (TurnLimit > 0 && CurrentTurn >= TurnLimit)
		{
			UStageSubsystem* StageSub = GetWorld()->GetSubsystem<UStageSubsystem>();
			if (StageSub)
			{
				StageSub->OnTurnLimitExceeded();
			}
			return;
		}

		OnMonsterTurnFinished.Broadcast();
		StartPawnTurn();
		return;
	}

	ACardGameActor* Monster = PendingMonsters[CurrentMonsterIndex].Get();
	if (!Monster || !Monster->Data)
	{
		// 유효하지 않은 몬스터 건너뛰기
		OnMonsterActionComplete();
		return;
	}

	UDMonster* MonData = Cast<UDMonster>(Monster->Data);
	if (!MonData || MonData->IsDead())
	{
		OnMonsterActionComplete();
		return;
	}

	MonsterAct(Monster);
}

void UTurnSubsystem::MonsterAct(ACardGameActor* Monster)
{
	UDMonster* MonData = Cast<UDMonster>(Monster->Data);
	if (!MonData) { OnMonsterActionComplete(); return; }

	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) { OnMonsterActionComplete(); return; }

	ASquareTile* MonsterTile = GetTileForActor(Monster);
	if (!MonsterTile) { OnMonsterActionComplete(); return; }

	UE_LOG(LogTurnSubsystem, Log, TEXT("  Monster Act: %s at (%d,%d)"),
		*MonData->GetName(), MonsterTile->GridPosition.X, MonsterTile->GridPosition.Y);

	if (bIsAlert)
	{
		// ── 경계 상태: 가장 가까운 아군 향해 이동/공격 ──
		ACardGameActor* Target = GetNearestActor(MonsterTile, CachedUsers);
		if (!Target) { OnMonsterActionComplete(); return; }

		ASquareTile* TargetTile = GetTileForActor(Target);
		if (!TargetTile) { OnMonsterActionComplete(); return; }

		const int32 Distance = GridSub->GetDistance(MonsterTile->GridPosition, TargetTile->GridPosition);

		// 공격 사거리 내면 즉시 공격
		if (MonData->Range > 0 && Distance <= MonData->Range)
		{
			UE_LOG(LogTurnSubsystem, Log, TEXT("    -> Attack (Range %d, Dist %d)"), MonData->Range, Distance);
			ExecuteAttack(Monster, Target, MonData->Attack, [this]() { OnMonsterActionComplete(); });
			return;
		}

		// 이동
		if (MonData->Movement > 0)
		{
			ASquareTile* BestTile = GetBestTileToward(MonsterTile, TargetTile, MonData->Movement);
			if (BestTile && BestTile != MonsterTile)
			{
				TArray<ASquareTile*> Path = GridSub->GetPath(
					MonsterTile->GridPosition, BestTile->GridPosition,
					MonData->Movement, UDMonster::StaticClass());
				Path = GridSub->SmoothPath(Path, MonsterTile->GridPosition, UDMonster::StaticClass());

				if (Path.Num() > 0)
				{
					UE_LOG(LogTurnSubsystem, Log, TEXT("    -> Move (%d tiles)"), Path.Num());
					Monster->MoveTo(Path, [this, Monster, MonData, GridSub]()
					{
						// 이동 후 공격 재시도
						ASquareTile* NewMonTile = GetTileForActor(Monster);
						ACardGameActor* NewTarget = GetNearestActor(NewMonTile, CachedUsers);
						if (NewTarget && NewMonTile)
						{
							ASquareTile* NewTargetTile = GetTileForActor(NewTarget);
							if (NewTargetTile)
							{
								const int32 NewDist = GridSub->GetDistance(
									NewMonTile->GridPosition, NewTargetTile->GridPosition);
								if (MonData->Range > 0 && NewDist <= MonData->Range)
								{
									UE_LOG(LogTurnSubsystem, Log, TEXT("    -> Post-move Attack"));
									ExecuteAttack(Monster, NewTarget, MonData->Attack,
										[this]() { OnMonsterActionComplete(); });
									return;
								}
							}
						}
						OnMonsterActionComplete();
					});
					return;
				}
			}
		}

		OnMonsterActionComplete();
	}
	else
	{
		// ── 배회 상태: 랜덤 이동 ──
		const int32 WanderRange = FMath::Min(2, MonData->Movement);
		if (WanderRange <= 0)
		{
			OnMonsterActionComplete();
			return;
		}

		TArray<ASquareTile*> Reachable = GridSub->GetReachableTiles(
			MonsterTile->GridPosition, WanderRange, UDMonster::StaticClass());

		// 현재 위치 제외
		Reachable.RemoveAll([MonsterTile](ASquareTile* T) { return T == MonsterTile; });

		if (Reachable.Num() == 0)
		{
			OnMonsterActionComplete();
			return;
		}

		ASquareTile* RandomTile = Reachable[FMath::RandRange(0, Reachable.Num() - 1)];
		TArray<ASquareTile*> Path = GridSub->GetPath(
			MonsterTile->GridPosition, RandomTile->GridPosition,
			WanderRange, UDMonster::StaticClass());
		Path = GridSub->SmoothPath(Path, MonsterTile->GridPosition, UDMonster::StaticClass());

		if (Path.Num() > 0)
		{
			UE_LOG(LogTurnSubsystem, Log, TEXT("    -> Wander (%d tiles)"), Path.Num());
			Monster->MoveTo(Path, [this]() { OnMonsterActionComplete(); });
		}
		else
		{
			OnMonsterActionComplete();
		}
	}
}

void UTurnSubsystem::OnMonsterActionComplete()
{
	CurrentMonsterIndex++;
	ProcessNextMonster();
}

// ────────────────────────────────────────────────────────────
// 공격 실행
// ────────────────────────────────────────────────────────────

void UTurnSubsystem::ExecuteAttack(ACardGameActor* Attacker, ACardGameActor* Target, int32 AttackPower, TFunction<void()> OnComplete)
{
	if (!Attacker || !Target || !Target->Data)
	{
		if (OnComplete) OnComplete();
		return;
	}

	// 공격/피격 애니메이션 병렬 완료 대기용 카운터
	auto Counter = MakeShared<int32>(0);
	TWeakObjectPtr<ACardGameActor> WeakTarget(Target);
	TWeakObjectPtr<UTurnSubsystem> WeakThis(this);

	auto CheckBothDone = [Counter, WeakTarget, WeakThis, OnComplete]()
	{
		(*Counter)++;
		if (*Counter < 2) return;

		ACardGameActor* TargetActor = WeakTarget.Get();
		if (!TargetActor || !WeakThis.IsValid())
		{
			if (OnComplete) OnComplete();
			return;
		}

		// 사망 체크
		bool bDead = false;
		if (UDPawn* P = Cast<UDPawn>(TargetActor->Data))
		{
			bDead = P->IsDead();
		}
		else if (UDMonster* M = Cast<UDMonster>(TargetActor->Data))
		{
			bDead = M->IsDead();
		}

		if (bDead)
		{
			UE_LOG(LogTurnSubsystem, Log, TEXT("    Target died!"));
			TargetActor->Die([WeakThis, OnComplete]()
			{
				if (WeakThis.IsValid())
				{
					UStageSubsystem* StageSub = WeakThis->GetWorld()->GetSubsystem<UStageSubsystem>();
					if (StageSub)
					{
						StageSub->CheckBattleResult();
					}
				}
				if (OnComplete) OnComplete();
			});
		}
		else
		{
			if (OnComplete) OnComplete();
		}
	};

	// 공격 애니메이션 시작
	Attacker->PerformAttack(Target, CheckBothDone);

	// 0.3초 후 피해 적용 + 피격 애니메이션
	FTimerHandle HitTimer;
	TWeakObjectPtr<ACardGameActor> WeakAttacker(Attacker);

	GetWorld()->GetTimerManager().SetTimer(HitTimer, [WeakTarget, WeakAttacker, AttackPower, CheckBothDone]()
	{
		ACardGameActor* TargetActor = WeakTarget.Get();
		if (!TargetActor) { CheckBothDone(); return; }

		// 피해 적용
		if (UDPawn* P = Cast<UDPawn>(TargetActor->Data))
		{
			P->TakeDamage(AttackPower);
		}
		else if (UDMonster* M = Cast<UDMonster>(TargetActor->Data))
		{
			M->TakeDamage(AttackPower);
		}

		// 피격 애니메이션
		ACardGameActor* AttackerActor = WeakAttacker.Get();
		TargetActor->ReceiveHit(AttackerActor, CheckBothDone);

	}, 0.3f, false);
}

// ────────────────────────────────────────────────────────────
// AI 헬퍼
// ────────────────────────────────────────────────────────────

bool UTurnSubsystem::CheckAlertState()
{
	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) return false;

	// 임의의 몬스터의 시야 범위 내에 아군이 있으면 전체 경계
	for (auto& MonWeak : PendingMonsters)
	{
		ACardGameActor* Monster = MonWeak.Get();
		if (!Monster) continue;

		UDMonster* MonData = Cast<UDMonster>(Monster->Data);
		if (!MonData) continue;

		ASquareTile* MonTile = GetTileForActor(Monster);
		if (!MonTile) continue;

		for (auto& UserWeak : CachedUsers)
		{
			ACardGameActor* User = UserWeak.Get();
			if (!User) continue;

			ASquareTile* UserTile = GetTileForActor(User);
			if (!UserTile) continue;

			const int32 Dist = GridSub->GetDistance(MonTile->GridPosition, UserTile->GridPosition);
			if (Dist <= MonData->Sight)
			{
				return true;
			}
		}
	}

	return false;
}

ACardGameActor* UTurnSubsystem::GetNearestActor(ASquareTile* From, const TArray<TWeakObjectPtr<ACardGameActor>>& Targets)
{
	if (!From) return nullptr;

	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) return nullptr;

	ACardGameActor* Nearest = nullptr;
	int32 MinDist = TNumericLimits<int32>::Max();

	for (auto& Weak : Targets)
	{
		ACardGameActor* Actor = Weak.Get();
		if (!Actor) continue;

		ASquareTile* ActorTile = GetTileForActor(Actor);
		if (!ActorTile) continue;

		const int32 Dist = GridSub->GetDistance(From->GridPosition, ActorTile->GridPosition);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			Nearest = Actor;
		}
	}

	return Nearest;
}

ASquareTile* UTurnSubsystem::GetBestTileToward(ASquareTile* From, ASquareTile* TargetTile, int32 Movement)
{
	if (!From || !TargetTile) return nullptr;

	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) return nullptr;

	TArray<ASquareTile*> Reachable = GridSub->GetReachableTiles(
		From->GridPosition, Movement, UDMonster::StaticClass());

	ASquareTile* Best = nullptr;
	int32 MinDist = TNumericLimits<int32>::Max();

	for (ASquareTile* Tile : Reachable)
	{
		const int32 Dist = GridSub->GetDistance(Tile->GridPosition, TargetTile->GridPosition);
		if (Dist < MinDist)
		{
			MinDist = Dist;
			Best = Tile;
		}
	}

	return Best;
}

ASquareTile* UTurnSubsystem::GetTileForActor(ACardGameActor* Actor) const
{
	if (!Actor || !Actor->CurrentSlot) return nullptr;

	return Cast<ASquareTile>(Actor->CurrentSlot->GetOwner());
}
