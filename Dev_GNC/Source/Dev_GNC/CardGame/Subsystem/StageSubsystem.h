// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "CardGameTypes.h"
#include "StageSubsystem.generated.h"

class ACardGameActor;
class ASquareTile;
class UDCard;
class UDPawn;
class UDMonster;
class UDObject;
class UTileMapPreset;
class UDataSubsystem;
struct FCardEffectRow;

// ── 델리게이트 ──────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSelectedChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBattleEnd, bool, bVictory);

/**
 * UStageSubsystem
 * 스테이지 진행, 액터 관리, 선택/이동, 카드 사용/효과를 관리하는 월드 서브시스템.
 * Unity StageManager (MonoSingleton) 대응.
 */
UCLASS()
class UStageSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ── 액터 관리 ──────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Stage")
	TMap<int32, TObjectPtr<ACardGameActor>> UserActors;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Stage")
	TMap<int32, TObjectPtr<ACardGameActor>> EnemyActors;

	// ── 선택 상태 ──────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Stage")
	TObjectPtr<ASquareTile> SelectedTile;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Stage")
	TObjectPtr<UDObject> SelectedObject;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Stage")
	bool bIsBusy = false;

	// ── 보상 ───────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Stage")
	int32 StageGoldReward = 0;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Stage")
	int32 StageExpReward = 0;

	// ── 타일 선택/이동 ─────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void SelectTile(ASquareTile* Tile);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void Deselect();

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void ShowMovementRange(FIntPoint Pos, int32 Movement, UClass* FriendlyClass = nullptr);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void ClearMovementHighlights();

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void RestoreMovementRange();

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void RefreshMovementRange();

	// ── 카드 범위 ──────────────────────────────────
	UFUNCTION(BlueprintPure, Category = "CardGame|Stage")
	int32 GetCardUseRange(UDCard* Card, ACardGameActor* Caster) const;

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void ShowCardRange(ASquareTile* CasterTile, UDCard* Card, ACardGameActor* Caster);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void ClearCardRange();

	UFUNCTION(BlueprintPure, Category = "CardGame|Stage")
	bool IsInCardRange(ASquareTile* Tile) const;

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void ShowRadiusPreview(ASquareTile* CenterTile, UDCard* Card);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void ClearRadiusPreview();

	// ── 카드 사용 ──────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void UseCardSelf(UDCard* Card, ACardGameActor* Caster, UDPawn* CardOwner = nullptr);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	bool TryUseCard(UDCard* Card, ACardGameActor* Caster, ASquareTile* TargetTile, UDPawn* CardOwner = nullptr);

	// ── 스테이지 ───────────────────────────────────
	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void LoadStage(UTileMapPreset* Preset);

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void CheckBattleResult();

	UFUNCTION(BlueprintCallable, Category = "CardGame|Stage")
	void OnTurnLimitExceeded();

	// ── 델리게이트 ─────────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "CardGame|Stage")
	FOnSelectedChanged OnSelectedChanged;

	UPROPERTY(BlueprintAssignable, Category = "CardGame|Stage")
	FOnBattleEnd OnBattleEnd;

private:
	// ── 하이라이트 추적 ────────────────────────────
	TSet<TObjectPtr<ASquareTile>> ReachableTiles;
	TSet<TObjectPtr<ASquareTile>> CardRangeTiles;
	TSet<TObjectPtr<ASquareTile>> RadiusPreviewTiles;

	// ── 카드 효과 (콜백 체인) ──────────────────────
	void ApplyCardEffects(UDCard* Card, ACardGameActor* Caster, ACardGameActor* Target, TFunction<void()> OnComplete);
	void ApplyWithRadius(UDCard* Card, ACardGameActor* Caster, ASquareTile* CenterTile, TFunction<void()> OnComplete);
	void ProcessNextEffect(UDCard* Card, int32 EffectIndex, ACardGameActor* Caster, ACardGameActor* Target, bool bIsHit, TFunction<void()> OnComplete);
	TArray<ACardGameActor*> GetRadiusTargets(ASquareTile* CenterTile, UDCard* Card);

	// ── 유틸리티 ────────────────────────────────────
	ASquareTile* GetTileForActor(ACardGameActor* Actor) const;
	ACardGameActor* GetActorOnTile(ASquareTile* Tile) const;
	bool IsEmptyTile(ASquareTile* Tile) const;
	int32 GetActorMovement(ACardGameActor* Actor) const;
	int32 GetEffectiveMovement(ACardGameActor* Actor) const;

	// ── 스폰 ───────────────────────────────────────
	void SpawnPawns(UTileMapPreset* Preset);
	void SpawnMonsters(UTileMapPreset* Preset);
	ACardGameActor* SpawnActorOnTile(ASquareTile* Tile);
};
