// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GridEntity.h"
#include "CardGameTypes.h"
#include "CardGameActor.generated.h"

class UDPawn;
class UDMonster;
class ASquareTile;
class UAnimMontage;
class UWidgetComponent;
class UHUDActorWidget;

/**
 * ACardGameActor
 * 전투 캐릭터 액터. 이동, 공격, 피격, 사망 애니메이션을 처리한다.
 * Unity Actor (TileEntity 상속) 대응.
 */
UCLASS(BlueprintType)
class ACardGameActor : public AGridEntity
{
	GENERATED_BODY()

public:
	ACardGameActor();

	// ── 초기화 ──────────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void InitFromPawn(UDPawn* Pawn);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void InitFromMonster(UDMonster* Monster);

	// ── 행동 (콜백 기반) ────────────────────────────
	void MoveTo(const TArray<ASquareTile*>& Path, TFunction<void()> OnComplete = nullptr);

	void PerformAttack(ACardGameActor* Target, TFunction<void()> OnComplete = nullptr);

	void ReceiveHit(ACardGameActor* Attacker, TFunction<void()> OnComplete = nullptr);

	void Evade(ACardGameActor* Attacker, TFunction<void()> OnComplete = nullptr);

	void Die(TFunction<void()> OnComplete = nullptr);

	// ── Tick ────────────────────────────────────────
	virtual void Tick(float DeltaTime) override;

	// ── 이동 상수 ──────────────────────────────────
	static constexpr float DefaultMoveSpeed = 300.f;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame")
	bool bIsMoving = false;

protected:
	// ── 컴포넌트 ────────────────────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CardGame")
	TObjectPtr<USkeletalMeshComponent> MeshComp;

	// ── 애니메이션 몽타주 (BP 서브클래스에서 설정) ──
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> GetHitMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> EvadeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> DieMontage;

	// ── HUD 위젯 (Phase 5-B) ───────────────────────
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CardGame|UI")
	TObjectPtr<UWidgetComponent> HUDWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "CardGame|UI")
	TSubclassOf<UHUDActorWidget> HUDWidgetClass;

private:
	// ── 이동 상태 ──────────────────────────────────
	float MoveSpeed = DefaultMoveSpeed;
	TArray<TObjectPtr<ASquareTile>> MoveWaypoints;
	int32 CurrentWaypointIndex = 0;
	TFunction<void()> OnMoveCompleteCallback;

	// ── 유틸리티 ────────────────────────────────────
	void FaceTarget(AActor* Target);
	void PlayMontageWithCallback(UAnimMontage* Montage, TFunction<void()> OnComplete);

	UFUNCTION()
	void HandleFloatingText(EFloatingTextType TextType, int32 Value);
};
