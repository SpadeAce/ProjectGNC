// Copyright GNC Project. All Rights Reserved.

#include "StageSubsystem.h"
#include "TurnSubsystem.h"
#include "GridSubsystem.h"
#include "DataSubsystem.h"
#include "DeckSubsystem.h"
#include "CardGameActor.h"
#include "SquareTile.h"
#include "GridSlotComponent.h"
#include "DPawn.h"
#include "DMonster.h"
#include "DCard.h"
#include "DObject.h"
#include "TileMapPreset.h"
#include "CardGameRowTypes.h"

DEFINE_LOG_CATEGORY_STATIC(LogStageSubsystem, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void UStageSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogStageSubsystem, Log, TEXT("UStageSubsystem Initialized"));
}

void UStageSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ────────────────────────────────────────────────────────────
// 타일 선택/이동
// ────────────────────────────────────────────────────────────

void UStageSubsystem::SelectTile(ASquareTile* Tile)
{
	if (!Tile || bIsBusy) return;

	UTurnSubsystem* TurnSub = GetWorld()->GetSubsystem<UTurnSubsystem>();
	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!TurnSub || !GridSub) return;

	// Case 1: 같은 타일 재클릭 → 해제
	if (SelectedTile == Tile)
	{
		Deselect();
		return;
	}

	// Case 2: 액터 선택된 상태에서 빈 타일 클릭 → 이동
	if (SelectedTile && SelectedObject)
	{
		ACardGameActor* SelectedActor = GetActorOnTile(SelectedTile);
		if (SelectedActor && ReachableTiles.Contains(Tile) && IsEmptyTile(Tile))
		{
			const int32 EffMov = GetEffectiveMovement(SelectedActor);
			TArray<ASquareTile*> Path = GridSub->GetPath(
				SelectedTile->GridPosition, Tile->GridPosition,
				EffMov, UDPawn::StaticClass());
			Path = GridSub->SmoothPath(Path, SelectedTile->GridPosition, UDPawn::StaticClass());

			if (Path.Num() > 0)
			{
				const int32 PathCost = Path.Num();
				const int32 Remaining = FMath::Max(0, EffMov - PathCost);

				bIsBusy = true;
				ClearMovementHighlights();

				ASquareTile* PrevTile = SelectedTile;
				SelectedActor->MoveTo(Path, [this, SelectedActor, Remaining, TurnSub]()
				{
					bIsBusy = false;
					TurnSub->OnActorMoved(SelectedActor, Remaining);

					// 이동 후 남은 이동력으로 범위 재표시
					ASquareTile* NewTile = GetTileForActor(SelectedActor);
					if (NewTile && Remaining > 0)
					{
						SelectedTile = NewTile;
						ShowMovementRange(NewTile->GridPosition, Remaining, UDPawn::StaticClass());
					}
					else if (NewTile)
					{
						SelectedTile = NewTile;
					}
				});
			}
			return;
		}
	}

	// Case 3: 새 타일 선택 (액터가 있는 타일)
	ACardGameActor* Actor = GetActorOnTile(Tile);
	if (Actor && Actor->Data)
	{
		ClearMovementHighlights();

		SelectedTile = Tile;
		SelectedObject = Actor->Data;
		Tile->SetSelected(true);

		// 아군 폰이면 이동 범위 표시
		if (TurnSub->IsPawnTurn() && Cast<UDPawn>(Actor->Data))
		{
			const int32 EffMov = GetEffectiveMovement(Actor);
			if (EffMov > 0)
			{
				ShowMovementRange(Tile->GridPosition, EffMov, UDPawn::StaticClass());
			}
		}

		OnSelectedChanged.Broadcast();
	}
}

void UStageSubsystem::Deselect()
{
	ClearMovementHighlights();
	ClearCardRange();
	ClearRadiusPreview();

	if (SelectedTile)
	{
		SelectedTile->SetSelected(false);
	}

	SelectedTile = nullptr;
	SelectedObject = nullptr;

	OnSelectedChanged.Broadcast();
}

void UStageSubsystem::ShowMovementRange(FIntPoint Pos, int32 Movement, UClass* FriendlyClass)
{
	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) return;

	TArray<ASquareTile*> Tiles = GridSub->GetReachableTiles(Pos, Movement, FriendlyClass);
	for (ASquareTile* Tile : Tiles)
	{
		if (Tile && Tile != SelectedTile)
		{
			Tile->SetHighlighted(true);
			ReachableTiles.Add(Tile);
		}
	}
}

void UStageSubsystem::ClearMovementHighlights()
{
	for (auto& Tile : ReachableTiles)
	{
		if (Tile.Get())
		{
			Tile->SetHighlighted(false);
		}
	}
	ReachableTiles.Reset();
}

void UStageSubsystem::RestoreMovementRange()
{
	if (!SelectedTile || !SelectedObject) return;

	ACardGameActor* Actor = GetActorOnTile(SelectedTile);
	if (!Actor) return;

	const int32 EffMov = GetEffectiveMovement(Actor);
	if (EffMov > 0)
	{
		ShowMovementRange(SelectedTile->GridPosition, EffMov, UDPawn::StaticClass());
	}
}

void UStageSubsystem::RefreshMovementRange()
{
	ClearMovementHighlights();
	RestoreMovementRange();
}

// ────────────────────────────────────────────────────────────
// 카드 범위
// ────────────────────────────────────────────────────────────

int32 UStageSubsystem::GetCardUseRange(UDCard* Card, ACardGameActor* Caster) const
{
	if (!Card || !Card->Data) return 0;

	int32 Range = Card->Data->Range;

	if (Caster && Caster->Data)
	{
		if (UDPawn* Pawn = Cast<UDPawn>(Caster->Data))
		{
			Range += Pawn->Range;
		}
	}

	return Range;
}

void UStageSubsystem::ShowCardRange(ASquareTile* CasterTile, UDCard* Card, ACardGameActor* Caster)
{
	if (!CasterTile || !Card) return;

	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) return;

	ClearCardRange();

	const int32 Range = GetCardUseRange(Card, Caster);
	TArray<ASquareTile*> Tiles = GridSub->GetTilesInRange(CasterTile->GridPosition, Range);

	for (ASquareTile* Tile : Tiles)
	{
		if (Tile && Tile->IsWalkable())
		{
			Tile->SetTarget(true);
			CardRangeTiles.Add(Tile);
		}
	}
}

void UStageSubsystem::ClearCardRange()
{
	for (auto& Tile : CardRangeTiles)
	{
		if (Tile.Get())
		{
			Tile->SetTarget(false);
		}
	}
	CardRangeTiles.Reset();
}

bool UStageSubsystem::IsInCardRange(ASquareTile* Tile) const
{
	return Tile && CardRangeTiles.Contains(const_cast<ASquareTile*>(Tile));
}

void UStageSubsystem::ShowRadiusPreview(ASquareTile* CenterTile, UDCard* Card)
{
	if (!CenterTile || !Card || !Card->Data) return;

	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) return;

	ClearRadiusPreview();

	const int32 Radius = Card->Data->Radius;
	if (Radius <= 0) return;

	TArray<ASquareTile*> Tiles = GridSub->GetTilesInRange(CenterTile->GridPosition, Radius);
	for (ASquareTile* Tile : Tiles)
	{
		if (Tile)
		{
			Tile->SetRadius(true);
			RadiusPreviewTiles.Add(Tile);
		}
	}
}

void UStageSubsystem::ClearRadiusPreview()
{
	for (auto& Tile : RadiusPreviewTiles)
	{
		if (Tile.Get())
		{
			Tile->SetRadius(false);
		}
	}
	RadiusPreviewTiles.Reset();
}

// ────────────────────────────────────────────────────────────
// 카드 사용
// ────────────────────────────────────────────────────────────

void UStageSubsystem::UseCardSelf(UDCard* Card, ACardGameActor* Caster, UDPawn* CardOwner)
{
	if (!Card || !Caster || bIsBusy) return;

	UTurnSubsystem* TurnSub = GetWorld()->GetSubsystem<UTurnSubsystem>();
	if (!TurnSub) return;

	// 비용 체크
	if (!TurnSub->HasEnoughSharedActingPower(Card->Data->EnergyCost)) return;

	TurnSub->ConsumeSharedActingPower(Card->Data->EnergyCost);

	// 탄약 소모
	if (CardOwner && Card->Data->AmmoCost > 0)
	{
		CardOwner->ConsumeAmmo(Card->Data->AmmoCost);
	}

	// 카드 소모 처리
	if (CardOwner)
	{
		CardOwner->UsePawnCard(Card);
	}
	else
	{
		if (UDeckSubsystem* DeckSub = GetWorld()->GetGameInstance()->GetSubsystem<UDeckSubsystem>())
		{
			DeckSub->UseCard(Card);
		}
	}

	bIsBusy = true;
	ApplyCardEffects(Card, Caster, Caster, [this]()
	{
		bIsBusy = false;
	});
}

bool UStageSubsystem::TryUseCard(UDCard* Card, ACardGameActor* Caster, ASquareTile* TargetTile, UDPawn* CardOwner)
{
	if (!Card || !Card->Data || !Caster || !TargetTile || bIsBusy) return false;

	UTurnSubsystem* TurnSub = GetWorld()->GetSubsystem<UTurnSubsystem>();
	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!TurnSub || !GridSub) return false;

	// 범위 체크
	ASquareTile* CasterTile = GetTileForActor(Caster);
	if (!CasterTile) return false;

	const int32 Range = GetCardUseRange(Card, Caster);
	const int32 Distance = GridSub->GetDistance(CasterTile->GridPosition, TargetTile->GridPosition);
	if (Distance > Range) return false;

	// 비용 체크
	if (!TurnSub->HasEnoughSharedActingPower(Card->Data->EnergyCost)) return false;

	// 탄약 체크
	if (CardOwner && Card->Data->AmmoCost > 0)
	{
		if (!CardOwner->HasEnoughAmmo(Card->Data->AmmoCost)) return false;
	}

	// 비용 소모
	TurnSub->ConsumeSharedActingPower(Card->Data->EnergyCost);
	if (CardOwner && Card->Data->AmmoCost > 0)
	{
		CardOwner->ConsumeAmmo(Card->Data->AmmoCost);
	}
	if (CardOwner)
	{
		CardOwner->UsePawnCard(Card);
	}
	else
	{
		if (UDeckSubsystem* DeckSub = GetWorld()->GetGameInstance()->GetSubsystem<UDeckSubsystem>())
		{
			DeckSub->UseCard(Card);
		}
	}

	bIsBusy = true;

	// Radius가 있으면 범위 효과, 없으면 단일 대상
	if (Card->Data->Radius > 0)
	{
		ApplyWithRadius(Card, Caster, TargetTile, [this]()
		{
			bIsBusy = false;
		});
	}
	else
	{
		ACardGameActor* TargetActor = GetActorOnTile(TargetTile);
		if (TargetActor)
		{
			ApplyCardEffects(Card, Caster, TargetActor, [this]()
			{
				bIsBusy = false;
			});
		}
		else
		{
			bIsBusy = false;
		}
	}

	return true;
}

// ────────────────────────────────────────────────────────────
// 카드 효과 (콜백 체인)
// ────────────────────────────────────────────────────────────

void UStageSubsystem::ApplyCardEffects(UDCard* Card, ACardGameActor* Caster, ACardGameActor* Target, TFunction<void()> OnComplete)
{
	if (!Card || !Card->Data || !Target)
	{
		if (OnComplete) OnComplete();
		return;
	}

	// 명중 판정: DPawn 공격자 vs DMonster 대상일 때만
	bool bIsHit = true;
	if (Target->Data && Caster->Data)
	{
		UDPawn* AtkPawn = Cast<UDPawn>(Caster->Data);
		UDMonster* DefMon = Cast<UDMonster>(Target->Data);
		if (AtkPawn && DefMon)
		{
			bIsHit = FMath::RandRange(0, 9999) < AtkPawn->Accuracy;
			if (!bIsHit)
			{
				UE_LOG(LogStageSubsystem, Log, TEXT("  Miss! (Accuracy: %d)"), AtkPawn->Accuracy);
				DefMon->NotifyMiss();
				// 미스 시 회피 애니메이션 후 완료
				Target->Evade(Caster, [OnComplete]()
				{
					if (OnComplete) OnComplete();
				});
				return;
			}
		}
	}

	ProcessNextEffect(Card, 0, Caster, Target, bIsHit, MoveTemp(OnComplete));
}

void UStageSubsystem::ApplyWithRadius(UDCard* Card, ACardGameActor* Caster, ASquareTile* CenterTile, TFunction<void()> OnComplete)
{
	TArray<ACardGameActor*> Targets = GetRadiusTargets(CenterTile, Card);

	if (Targets.Num() == 0)
	{
		if (OnComplete) OnComplete();
		return;
	}

	// 순차적으로 각 대상에게 효과 적용
	auto TargetIndex = MakeShared<int32>(0);
	TFunction<void()> ProcessNext;
	ProcessNext = [this, Card, Caster, Targets, TargetIndex, OnComplete, &ProcessNext]()
	{
		if (*TargetIndex >= Targets.Num())
		{
			if (OnComplete) OnComplete();
			return;
		}

		ACardGameActor* Target = Targets[*TargetIndex];
		(*TargetIndex)++;

		ApplyCardEffects(Card, Caster, Target, ProcessNext);
	};

	// 재귀 호출을 위해 캡처 가능하도록 shared_ptr로 감싸기
	auto SharedProcessNext = MakeShared<TFunction<void()>>();
	*SharedProcessNext = [this, Card, Caster, Targets, TargetIndex, OnComplete, SharedProcessNext]()
	{
		if (*TargetIndex >= Targets.Num())
		{
			if (OnComplete) OnComplete();
			return;
		}

		ACardGameActor* Target = Targets[*TargetIndex];
		(*TargetIndex)++;

		ApplyCardEffects(Card, Caster, Target, *SharedProcessNext);
	};

	(*SharedProcessNext)();
}

void UStageSubsystem::ProcessNextEffect(UDCard* Card, int32 EffectIndex, ACardGameActor* Caster, ACardGameActor* Target, bool bIsHit, TFunction<void()> OnComplete)
{
	if (!Card || !Card->Data || EffectIndex >= Card->Data->EffectId.Num())
	{
		if (OnComplete) OnComplete();
		return;
	}

	// DataSubsystem에서 효과 조회
	UDataSubsystem* DataSub = GetWorld()->GetGameInstance()->GetSubsystem<UDataSubsystem>();
	if (!DataSub)
	{
		if (OnComplete) OnComplete();
		return;
	}

	const int32 EffId = Card->Data->EffectId[EffectIndex];
	const int32 Value = Card->Data->EffectValue[EffectIndex];
	const FCardEffectRow* EffRow = DataSub->GetCardEffectData(EffId);

	if (!EffRow)
	{
		// 효과를 찾지 못하면 다음으로
		ProcessNextEffect(Card, EffectIndex + 1, Caster, Target, bIsHit, MoveTemp(OnComplete));
		return;
	}

	UTurnSubsystem* TurnSub = GetWorld()->GetSubsystem<UTurnSubsystem>();

	UE_LOG(LogStageSubsystem, Log, TEXT("  Effect[%d]: Type=%d Value=%d"),
		EffectIndex, static_cast<int32>(EffRow->EffectType), Value);

	switch (EffRow->EffectType)
	{
	case ECardEffectType::Damage:
	{
		if (!bIsHit)
		{
			ProcessNextEffect(Card, EffectIndex + 1, Caster, Target, bIsHit, MoveTemp(OnComplete));
			return;
		}

		// 공격 → 피격 → 사망체크 → 다음 효과
		Caster->PerformAttack(Target, [this, Card, EffectIndex, Caster, Target, bIsHit, Value, OnComplete = MoveTemp(OnComplete)]() mutable
		{
			// 피해 적용
			if (UDPawn* P = Cast<UDPawn>(Target->Data)) P->TakeDamage(Value);
			else if (UDMonster* M = Cast<UDMonster>(Target->Data)) M->TakeDamage(Value);

			Target->ReceiveHit(Caster, [this, Card, EffectIndex, Caster, Target, bIsHit, OnComplete = MoveTemp(OnComplete)]() mutable
			{
				// 사망 체크
				bool bDead = false;
				if (UDPawn* P = Cast<UDPawn>(Target->Data)) bDead = P->IsDead();
				else if (UDMonster* M = Cast<UDMonster>(Target->Data)) bDead = M->IsDead();

				if (bDead)
				{
					// 보상 누적 (몬스터 처치 시)
					if (UDMonster* M = Cast<UDMonster>(Target->Data))
					{
						if (M->Data)
						{
							StageGoldReward += M->Data->Gold;
							StageExpReward += M->Data->Exp;
						}
					}

					Target->Die([this, Card, EffectIndex, Caster, Target, bIsHit, OnComplete = MoveTemp(OnComplete)]() mutable
					{
						CheckBattleResult();
						ProcessNextEffect(Card, EffectIndex + 1, Caster, Target, bIsHit, MoveTemp(OnComplete));
					});
				}
				else
				{
					ProcessNextEffect(Card, EffectIndex + 1, Caster, Target, bIsHit, MoveTemp(OnComplete));
				}
			});
		});
		return;
	}

	case ECardEffectType::Heal:
	{
		if (UDPawn* P = Cast<UDPawn>(Target->Data)) P->Heal(Value);
		else if (UDMonster* M = Cast<UDMonster>(Target->Data)) M->Heal(Value);
		break;
	}

	case ECardEffectType::Shield:
	{
		if (UDPawn* P = Cast<UDPawn>(Target->Data)) P->AddShield(Value);
		else if (UDMonster* M = Cast<UDMonster>(Target->Data)) M->AddShield(Value);
		break;
	}

	case ECardEffectType::RestoreAction:
	{
		if (TurnSub) TurnSub->AddSharedActingPower(Value);
		break;
	}

	case ECardEffectType::Reload:
	{
		if (UDPawn* P = Cast<UDPawn>(Target->Data)) P->RestoreAmmo();
		break;
	}

	case ECardEffectType::DrawCard:
	{
		if (UDPawn* P = Cast<UDPawn>(Target->Data)) P->DrawCards(Value);
		if (UDeckSubsystem* DeckSub = GetWorld()->GetGameInstance()->GetSubsystem<UDeckSubsystem>())
		{
			DeckSub->DrawCards(Value);
		}
		break;
	}

	case ECardEffectType::BuffAttack:
	case ECardEffectType::BuffArmor:
	case ECardEffectType::BuffMovement:
	{
		if (UDPawn* P = Cast<UDPawn>(Target->Data))
			P->ApplyBuff(EffRow->EffectType, Value, EffRow->Duration);
		else if (UDMonster* M = Cast<UDMonster>(Target->Data))
			M->ApplyBuff(EffRow->EffectType, Value, EffRow->Duration);

		// 이동력 버프면 잔여 이동력 조정
		if (EffRow->EffectType == ECardEffectType::BuffMovement && TurnSub)
		{
			TurnSub->AdjustRemainingMovement(Cast<ACardGameActor>(Target), Value);
		}
		break;
	}

	case ECardEffectType::DebuffAttack:
	case ECardEffectType::DebuffArmor:
	case ECardEffectType::DebuffMovement:
	{
		if (!bIsHit) break; // 미스 시 디버프 적용 안 함

		if (UDPawn* P = Cast<UDPawn>(Target->Data))
			P->ApplyBuff(EffRow->EffectType, -Value, EffRow->Duration);
		else if (UDMonster* M = Cast<UDMonster>(Target->Data))
			M->ApplyBuff(EffRow->EffectType, -Value, EffRow->Duration);

		// 이동력 디버프면 잔여 이동력 조정
		if (EffRow->EffectType == ECardEffectType::DebuffMovement && TurnSub)
		{
			TurnSub->AdjustRemainingMovement(Cast<ACardGameActor>(Target), -Value);
		}
		break;
	}

	default:
		break;
	}

	// 즉시 효과는 바로 다음으로
	ProcessNextEffect(Card, EffectIndex + 1, Caster, Target, bIsHit, MoveTemp(OnComplete));
}

TArray<ACardGameActor*> UStageSubsystem::GetRadiusTargets(ASquareTile* CenterTile, UDCard* Card)
{
	TArray<ACardGameActor*> Result;
	if (!CenterTile || !Card || !Card->Data) return Result;

	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) return Result;

	const int32 Radius = Card->Data->Radius;
	TArray<ASquareTile*> Tiles = GridSub->GetTilesInRange(CenterTile->GridPosition, Radius);

	for (ASquareTile* Tile : Tiles)
	{
		ACardGameActor* Actor = GetActorOnTile(Tile);
		if (!Actor) continue;

		// 대상 타입에 따라 필터링
		switch (Card->Data->Target)
		{
		case ETargetType::Enemy:
			if (Cast<UDMonster>(Actor->Data)) Result.Add(Actor);
			break;
		case ETargetType::Ally:
			if (Cast<UDPawn>(Actor->Data)) Result.Add(Actor);
			break;
		case ETargetType::Ground:
			Result.Add(Actor); // Ground는 모든 대상
			break;
		default:
			break;
		}
	}

	return Result;
}

// ────────────────────────────────────────────────────────────
// 스테이지
// ────────────────────────────────────────────────────────────

void UStageSubsystem::LoadStage(UTileMapPreset* Preset)
{
	if (!Preset) return;

	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	UTurnSubsystem* TurnSub = GetWorld()->GetSubsystem<UTurnSubsystem>();
	if (!GridSub || !TurnSub) return;

	UE_LOG(LogStageSubsystem, Log, TEXT("LoadStage: %dx%d, TurnLimit=%d"),
		Preset->Width, Preset->Height, Preset->TurnLimit);

	// 맵 생성
	GridSub->GenerateTileMapFromPreset(Preset);

	// 턴 리밋 설정
	TurnSub->SetTurnLimit(Preset->TurnLimit);

	// 보상 초기화
	StageGoldReward = 0;
	StageExpReward = 0;

	// 액터 스폰
	SpawnMonsters(Preset);
	SpawnPawns(Preset);

	// 글로벌 덱 초기화 (셔플 + 초기 핸드 드로우)
	if (UDeckSubsystem* DeckSub = GetWorld()->GetGameInstance()->GetSubsystem<UDeckSubsystem>())
	{
		DeckSub->InitStage();
	}

	// 게임 시작
	TurnSub->StartGame();
}

void UStageSubsystem::SpawnPawns(UTileMapPreset* Preset)
{
	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub || !Preset) return;

	// DeckSubsystem에서 편성된 폰 목록 취득
	UDeckSubsystem* DeckSub = GetWorld()->GetGameInstance()->GetSubsystem<UDeckSubsystem>();
	if (!DeckSub) return;

	TArray<UDPawn*> DeckPawns = DeckSub->GetActivePawns();
	if (DeckPawns.Num() == 0)
	{
		UE_LOG(LogStageSubsystem, Warning, TEXT("SpawnPawns: No active pawns in DeckSubsystem"));
		return;
	}

	TArray<FTilePresetData> SpawnPoints = GridSub->GetSpawnPoints(Preset, ESpawnPointType::Player);
	const int32 MaxPawns = Preset->MaxPawnCount > 0 ? Preset->MaxPawnCount : SpawnPoints.Num();
	const int32 PawnCount = FMath::Min3(MaxPawns, SpawnPoints.Num(), DeckPawns.Num());

	int32 ActorIndex = 0;
	int32 PawnIndex = 0;

	for (const FTilePresetData& Sp : SpawnPoints)
	{
		if (ActorIndex >= PawnCount) break;

		ASquareTile* Tile = GridSub->GetTile(Sp.Position);
		if (!Tile || !Tile->Slot) continue;

		UDPawn* Pawn = DeckPawns[PawnIndex];

		// ActorLink에서 액터 클래스 resolve
		TSubclassOf<ACardGameActor> ActorClass = nullptr;
		if (Pawn->Data && !Pawn->Data->ActorLink.IsNull())
		{
			ActorClass = Pawn->Data->ActorLink.LoadSynchronous();
		}

		ACardGameActor* Actor = SpawnActorOnTile(Tile, ActorClass);
		if (!Actor) continue;
		Pawn->ResetStats();
		Pawn->BuildCardPool();
		Pawn->InitHand();
		Actor->InitFromPawn(Pawn);

		UserActors.Add(ActorIndex, Actor);
		ActorIndex++;
		PawnIndex++;

		UE_LOG(LogStageSubsystem, Log, TEXT("  Spawned Pawn %s at (%d,%d)"),
			*Pawn->CodeName, Sp.Position.X, Sp.Position.Y);
	}
}

void UStageSubsystem::SpawnMonsters(UTileMapPreset* Preset)
{
	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub || !Preset) return;

	TArray<FTilePresetData> SpawnPoints = GridSub->GetSpawnPoints(Preset, ESpawnPointType::Enemy);

	UDataSubsystem* DataSub = GetWorld()->GetGameInstance()->GetSubsystem<UDataSubsystem>();
	if (!DataSub) return;

	int32 ActorIndex = 0;

	for (const FTilePresetData& Sp : SpawnPoints)
	{
		const int32 MonsterId = Sp.SpawnId > 0 ? Sp.SpawnId : 1;

		ASquareTile* Tile = GridSub->GetTile(Sp.Position);
		if (!Tile || !Tile->Slot) continue;

		// ActorLink에서 액터 클래스 resolve
		TSubclassOf<ACardGameActor> ActorClass = nullptr;
		const FMonsterDataRow* MonsterData = DataSub->GetMonsterData(MonsterId);
		if (MonsterData && !MonsterData->ActorLink.IsNull())
		{
			ActorClass = MonsterData->ActorLink.LoadSynchronous();
		}

		ACardGameActor* Actor = SpawnActorOnTile(Tile, ActorClass);
		if (!Actor) continue;

		// 몬스터 데이터 초기화
		UDMonster* Monster = NewObject<UDMonster>(this);
		if (MonsterData)
		{
			Monster->InitFromData(MonsterData);
		}
		else
		{
			Monster->InitFromId(MonsterId, DataSub);
		}
		Actor->InitFromMonster(Monster);

		EnemyActors.Add(ActorIndex, Actor);
		ActorIndex++;

		UE_LOG(LogStageSubsystem, Log, TEXT("  Spawned Monster %d at (%d,%d)"),
			MonsterId, Sp.Position.X, Sp.Position.Y);
	}
}

ACardGameActor* UStageSubsystem::SpawnActorOnTile(ASquareTile* Tile, TSubclassOf<ACardGameActor> ActorClass)
{
	if (!Tile || !Tile->Slot) return nullptr;

	if (!ActorClass)
	{
		ActorClass = ACardGameActor::StaticClass();
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ACardGameActor* Actor = GetWorld()->SpawnActor<ACardGameActor>(
		ActorClass,
		Tile->GetActorLocation(),
		FRotator::ZeroRotator,
		Params);

	if (Actor)
	{
		Tile->Slot->SetEntity(Actor);
	}

	return Actor;
}

void UStageSubsystem::CheckBattleResult()
{
	// 전체 적 사망 체크
	bool bAllEnemiesDead = true;
	for (auto& Pair : EnemyActors)
	{
		if (ACardGameActor* Actor = Pair.Value)
		{
			if (UDMonster* M = Cast<UDMonster>(Actor->Data))
			{
				if (!M->IsDead())
				{
					bAllEnemiesDead = false;
					break;
				}
			}
		}
	}

	if (bAllEnemiesDead && EnemyActors.Num() > 0)
	{
		StageGoldReward += 100; // 승리 보너스
		UE_LOG(LogStageSubsystem, Log, TEXT("=== VICTORY! Gold: %d, Exp: %d ==="), StageGoldReward, StageExpReward);
		OnBattleEnd.Broadcast(true);
		return;
	}

	// 전체 아군 사망 체크
	bool bAllUsersDead = true;
	for (auto& Pair : UserActors)
	{
		if (ACardGameActor* Actor = Pair.Value)
		{
			if (UDPawn* P = Cast<UDPawn>(Actor->Data))
			{
				if (!P->IsDead())
				{
					bAllUsersDead = false;
					break;
				}
			}
		}
	}

	if (bAllUsersDead && UserActors.Num() > 0)
	{
		UE_LOG(LogStageSubsystem, Log, TEXT("=== DEFEAT! ==="));
		OnBattleEnd.Broadcast(false);
	}
}

void UStageSubsystem::OnTurnLimitExceeded()
{
	UE_LOG(LogStageSubsystem, Log, TEXT("=== DEFEAT (Turn Limit Exceeded) ==="));
	OnBattleEnd.Broadcast(false);
}

// ────────────────────────────────────────────────────────────
// 유틸리티
// ────────────────────────────────────────────────────────────

ASquareTile* UStageSubsystem::GetTileForActor(ACardGameActor* Actor) const
{
	if (!Actor || !Actor->CurrentSlot) return nullptr;
	return Cast<ASquareTile>(Actor->CurrentSlot->GetOwner());
}

ACardGameActor* UStageSubsystem::GetActorOnTile(ASquareTile* Tile) const
{
	if (!Tile || !Tile->Slot) return nullptr;
	return Cast<ACardGameActor>(Tile->Slot->SlotEntity);
}

bool UStageSubsystem::IsEmptyTile(ASquareTile* Tile) const
{
	if (!Tile || !Tile->Slot) return false;
	return !Tile->Slot->IsOccupied();
}

int32 UStageSubsystem::GetActorMovement(ACardGameActor* Actor) const
{
	if (!Actor || !Actor->Data) return 0;

	if (UDPawn* P = Cast<UDPawn>(Actor->Data)) return P->Movement;
	if (UDMonster* M = Cast<UDMonster>(Actor->Data)) return M->Movement;
	return 0;
}

int32 UStageSubsystem::GetEffectiveMovement(ACardGameActor* Actor) const
{
	if (!Actor) return 0;

	UTurnSubsystem* TurnSub = GetWorld()->GetSubsystem<UTurnSubsystem>();
	if (!TurnSub) return GetActorMovement(Actor);

	const int32 Remaining = TurnSub->GetRemainingMovement(Actor);
	if (Remaining >= 0) return Remaining; // 이동 기록 있음

	return GetActorMovement(Actor); // 아직 이동 안 함, 기본 이동력
}
