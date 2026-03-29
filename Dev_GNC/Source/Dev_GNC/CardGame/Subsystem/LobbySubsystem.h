// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LobbySubsystem.generated.h"

class UDCard;
class UDPawn;
class UDataSubsystem;
class UTextSubsystem;

// ── 델리게이트 ──────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnShopListChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecruitListChanged);

/**
 * ULobbySubsystem
 * 상점 카드 목록과 모집 폰 목록을 생성·관리하는 게임인스턴스 서브시스템.
 * Unity LobbyManager (MonoSingleton) 대응.
 */
UCLASS()
class ULobbySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ══════════════════════════════════════════════
	// A. 상점 시스템
	// ══════════════════════════════════════════════

	static constexpr int32 ShopMaxItemCount = 10;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Lobby")
	int32 ShopLevel = 1;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Lobby")
	TArray<TObjectPtr<UDCard>> ShopCards;

	/** 현재 ShopLevel 기준으로 상점 카드 목록 생성 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Lobby")
	void GenerateShopList();

	/** 상점 레벨업. 최대 레벨이면 false 반환 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Lobby")
	bool LevelUpShop();

	/** 구매 후 카드 제거 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Lobby")
	void RemoveShopCard(UDCard* Card);

	// ══════════════════════════════════════════════
	// B. 모집 시스템
	// ══════════════════════════════════════════════

	static constexpr int32 RecruitSlotCount = 3;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Lobby")
	TArray<TObjectPtr<UDPawn>> RecruitPawns;

	/** 전체 PawnData에서 랜덤 3개 폰 생성 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Lobby")
	void GenerateRecruitList();

	/** 모집 후 폰 제거 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Lobby")
	void RemoveRecruitPawn(UDPawn* Pawn);

	// ══════════════════════════════════════════════
	// C. 리셋
	// ══════════════════════════════════════════════

	/** 스테이지 종료 후 호출. 목록만 비움 (다음 로비 진입 시 재생성) */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Lobby")
	void ResetForNextStage();

	/** 뉴게임 시 호출. 레벨 + 목록 전체 초기화 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Lobby")
	void ResetAll();

	// ══════════════════════════════════════════════
	// D. 로비 진입
	// ══════════════════════════════════════════════

	/** 로비 진입 시 호출. 목록이 비어있으면 자동 생성 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Lobby")
	void EnsureLobbyData();

	// ── 델리게이트 ───────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "CardGame|Lobby")
	FOnShopListChanged OnShopListChanged;

	UPROPERTY(BlueprintAssignable, Category = "CardGame|Lobby")
	FOnRecruitListChanged OnRecruitListChanged;

private:
	TWeakObjectPtr<UDataSubsystem> DataSubRef;
	TWeakObjectPtr<UTextSubsystem> TextSubRef;

	void CacheSubsystems();
};
