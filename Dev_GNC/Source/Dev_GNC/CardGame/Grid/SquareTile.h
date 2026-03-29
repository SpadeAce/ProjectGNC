// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CardGameTypes.h"
#include "SquareTile.generated.h"

class UGridSlotComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTileStateChanged, ASquareTile*, Tile, ETileState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTileClicked2, ASquareTile*, Tile);

/**
 * ASquareTile
 * 사각형 타일 액터. Unity SquareTile (MonoBehaviour) 대응.
 */
UCLASS(BlueprintType)
class ASquareTile : public AActor
{
	GENERATED_BODY()

public:
	ASquareTile();

	// ── 초기화 ──
	void Init(int32 InTileId, FIntPoint InGridPosition, ETileType InType = ETileType::Ground);

	// ── 프로퍼티 ──
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") FIntPoint GridPosition;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") int32 TileId = 0;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") ETileType TileType = ETileType::Empty;
	UPROPERTY(BlueprintReadOnly, Category = "CardGame") ETileState TileState = ETileState::Normal;

	// ── 슬롯 ──
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "CardGame")
	TObjectPtr<UGridSlotComponent> Slot;

	// ── 상태/조회 ──
	UFUNCTION(BlueprintPure, Category = "CardGame")
	bool IsWalkable() const;

	UFUNCTION(BlueprintPure, Category = "CardGame")
	bool IsSelected() const { return TileState == ETileState::Selected; }

	// ── 상태 변경 ──
	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetTileType(ETileType InType);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetState(ETileState NewState);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void ToggleSelection();

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetSelected(bool bSelected);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetHighlighted(bool bHighlighted);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetTarget(bool bTarget);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void SetRadius(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "CardGame")
	void Clear();

	// ── 델리게이트 ──
	UPROPERTY(BlueprintAssignable, Category = "CardGame")
	FOnTileStateChanged OnStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "CardGame")
	FOnTileClicked2 OnTileClicked;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> TileMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> SelectedOverlay;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> HighlightedOverlay;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> TargetOverlay;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UStaticMeshComponent> RadiusOverlay;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visual")
	TMap<ETileType, TSoftObjectPtr<UStaticMesh>> TileMeshByType;

private:
	void UpdateVisuals();
	void ApplyTileTypeMesh();
};
