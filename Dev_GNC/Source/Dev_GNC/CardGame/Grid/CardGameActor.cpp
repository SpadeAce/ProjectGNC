// Copyright GNC Project. All Rights Reserved.

#include "CardGameActor.h"
#include "SquareTile.h"
#include "GridSlotComponent.h"
#include "DPawn.h"
#include "DMonster.h"
#include "HUDActorWidget.h"
#include "CardGameHUD.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

ACardGameActor::ACardGameActor()
{
	PrimaryActorTick.bCanEverTick = false; // Tick은 이동 시에만 활성화

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(SceneRoot);

	// HUD 위젯 컴포넌트 (HP바/이름)
	HUDWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HUDWidget"));
	HUDWidgetComp->SetupAttachment(SceneRoot);
	HUDWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HUDWidgetComp->SetDrawAtDesiredSize(true);
	HUDWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
}

// ────────────────────────────────────────────────────────────
// 초기화
// ────────────────────────────────────────────────────────────

void ACardGameActor::InitFromPawn(UDPawn* Pawn)
{
	if (!Pawn) return;

	SetData(Pawn);
	Pawn->OnFloatingText.AddDynamic(this, &ACardGameActor::HandleFloatingText);

	// HUD 위젯 초기화
	if (HUDWidgetClass && HUDWidgetComp)
	{
		HUDWidgetComp->SetWidgetClass(HUDWidgetClass);
		HUDWidgetComp->InitWidget();
		if (UHUDActorWidget* HUDWidget = Cast<UHUDActorWidget>(HUDWidgetComp->GetWidget()))
		{
			HUDWidget->InitFromPawn(Pawn);
		}
	}
}

void ACardGameActor::InitFromMonster(UDMonster* Monster)
{
	if (!Monster) return;

	SetData(Monster);
	Monster->OnFloatingText.AddDynamic(this, &ACardGameActor::HandleFloatingText);

	// HUD 위젯 초기화
	if (HUDWidgetClass && HUDWidgetComp)
	{
		HUDWidgetComp->SetWidgetClass(HUDWidgetClass);
		HUDWidgetComp->InitWidget();
		if (UHUDActorWidget* HUDWidget = Cast<UHUDActorWidget>(HUDWidgetComp->GetWidget()))
		{
			HUDWidget->InitFromMonster(Monster);
		}
	}
}

// ────────────────────────────────────────────────────────────
// 이동 (Tick 기반 보간)
// ────────────────────────────────────────────────────────────

void ACardGameActor::MoveTo(const TArray<ASquareTile*>& Path, TFunction<void()> OnComplete)
{
	if (Path.Num() == 0)
	{
		if (OnComplete) OnComplete();
		return;
	}

	// 현재 슬롯에서 분리
	if (CurrentSlot)
	{
		CurrentSlot->ClearEntity();
	}

	MoveWaypoints.Reset();
	for (ASquareTile* Tile : Path)
	{
		MoveWaypoints.Add(Tile);
	}
	CurrentWaypointIndex = 0;
	OnMoveCompleteCallback = MoveTemp(OnComplete);
	bIsMoving = true;

	// 첫 웨이포인트를 향해 회전
	if (MoveWaypoints.IsValidIndex(0))
	{
		FaceTarget(MoveWaypoints[0]);
	}

	SetActorTickEnabled(true);
}

void ACardGameActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsMoving) return;

	if (!MoveWaypoints.IsValidIndex(CurrentWaypointIndex))
	{
		bIsMoving = false;
		SetActorTickEnabled(false);
		return;
	}

	const FVector TargetPos = MoveWaypoints[CurrentWaypointIndex]->GetActorLocation();
	const FVector CurrentPos = GetActorLocation();
	const FVector NewPos = FMath::VInterpConstantTo(CurrentPos, TargetPos, DeltaTime, MoveSpeed);
	SetActorLocation(NewPos);

	if (FVector::DistSquared(NewPos, TargetPos) < 1.f)
	{
		SetActorLocation(TargetPos);
		CurrentWaypointIndex++;

		if (CurrentWaypointIndex >= MoveWaypoints.Num())
		{
			// 이동 완료: 마지막 타일 슬롯에 부착
			bIsMoving = false;
			SetActorTickEnabled(false);

			ASquareTile* FinalTile = MoveWaypoints.Last();
			if (FinalTile && FinalTile->Slot)
			{
				FinalTile->Slot->SetEntity(this);
			}

			MoveWaypoints.Reset();

			if (OnMoveCompleteCallback)
			{
				auto Callback = MoveTemp(OnMoveCompleteCallback);
				Callback();
			}
		}
		else
		{
			// 다음 웨이포인트로 회전
			FaceTarget(MoveWaypoints[CurrentWaypointIndex]);
		}
	}
}

// ────────────────────────────────────────────────────────────
// 애니메이션 행동
// ────────────────────────────────────────────────────────────

void ACardGameActor::PerformAttack(ACardGameActor* Target, TFunction<void()> OnComplete)
{
	if (Target)
	{
		FaceTarget(Target);
	}

	PlayMontageWithCallback(AttackMontage, MoveTemp(OnComplete));
}

void ACardGameActor::ReceiveHit(ACardGameActor* Attacker, TFunction<void()> OnComplete)
{
	if (Attacker)
	{
		FaceTarget(Attacker);
	}

	PlayMontageWithCallback(GetHitMontage, MoveTemp(OnComplete));
}

void ACardGameActor::Evade(ACardGameActor* Attacker, TFunction<void()> OnComplete)
{
	if (Attacker)
	{
		FaceTarget(Attacker);
	}

	PlayMontageWithCallback(EvadeMontage, MoveTemp(OnComplete));
}

void ACardGameActor::Die(TFunction<void()> OnComplete)
{
	// 슬롯에서 분리
	if (CurrentSlot)
	{
		CurrentSlot->ClearEntity();
		CurrentSlot = nullptr;
	}

	auto PostDie = [this, OnComplete = MoveTemp(OnComplete)]()
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);

		if (OnComplete) OnComplete();
	};

	PlayMontageWithCallback(DieMontage, MoveTemp(PostDie));
}

// ────────────────────────────────────────────────────────────
// 유틸리티
// ────────────────────────────────────────────────────────────

void ACardGameActor::FaceTarget(AActor* Target)
{
	if (!Target) return;

	FVector Direction = Target->GetActorLocation() - GetActorLocation();
	Direction.Z = 0.f; // XY 평면에서만 회전

	if (!Direction.IsNearlyZero())
	{
		SetActorRotation(Direction.Rotation());
	}
}

void ACardGameActor::PlayMontageWithCallback(UAnimMontage* Montage, TFunction<void()> OnComplete)
{
	if (!Montage || !MeshComp)
	{
		if (OnComplete) OnComplete();
		return;
	}

	UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
	if (!AnimInst)
	{
		if (OnComplete) OnComplete();
		return;
	}

	const float Duration = AnimInst->Montage_Play(Montage);
	if (Duration <= 0.f)
	{
		if (OnComplete) OnComplete();
		return;
	}

	FOnMontageEnded EndDelegate;
	EndDelegate.BindLambda([OnComplete = MoveTemp(OnComplete)](UAnimMontage*, bool)
	{
		if (OnComplete) OnComplete();
	});
	AnimInst->Montage_SetEndDelegate(EndDelegate, Montage);
}

void ACardGameActor::HandleFloatingText(EFloatingTextType TextType, int32 Value)
{
	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	ACardGameHUD* HUD = PC ? Cast<ACardGameHUD>(PC->GetHUD()) : nullptr;
	if (!HUD) return;

	FString Text;
	switch (TextType)
	{
	case EFloatingTextType::Damage: Text = FString::Printf(TEXT("-%d"), Value); break;
	case EFloatingTextType::Heal:   Text = FString::Printf(TEXT("+%d"), Value); break;
	case EFloatingTextType::Shield: Text = FString::Printf(TEXT("+%d"), Value); break;
	case EFloatingTextType::Miss:   Text = TEXT("MISS"); break;
	case EFloatingTextType::Block:  Text = TEXT("BLOCK"); break;
	default: Text = FString::FromInt(Value); break;
	}

	HUD->ShowFloatingText(TextType, Text, GetActorLocation() + FVector(0.f, 0.f, 150.f));
}
