// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DeckSubsystem.generated.h"

class UDPawn;
class UDCard;

// ── 델리게이트 ──────────────────────────────────────────────
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeckHandChanged);

/**
 * UDeckSubsystem
 * Pawn 편성 슬롯과 글로벌 카드 덱(드로우/핸드/소모)을 관리하는 게임인스턴스 서브시스템.
 * Unity DeckManager (MonoSingleton) 대응.
 */
UCLASS()
class UDeckSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// ══════════════════════════════════════════════
	// A. Pawn 편성 슬롯 관리
	// ══════════════════════════════════════════════

	static constexpr int32 DeckSlotCount = 5;
	static constexpr int32 InitialOpenSlots = 2;

	/** 슬롯 1개 확장. 최대 DeckSlotCount까지. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	bool ExpandSlot();

	/** 폰을 지정 슬롯에 배치 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	bool AssignPawnToDeck(UDPawn* Pawn, int32 SlotIndex);

	/** 슬롯 비우기 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	void UnassignPawnFromDeck(int32 SlotIndex);

	/** 빈 슬롯 인덱스 반환 (없으면 -1) */
	UFUNCTION(BlueprintPure, Category = "CardGame|Deck")
	int32 FindEmptySlotIndex() const;

	/** 슬롯의 폰 조회 */
	UFUNCTION(BlueprintPure, Category = "CardGame|Deck")
	UDPawn* GetDeckPawn(int32 SlotIndex) const;

	/** null이 아닌 편성 폰 목록 반환 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	TArray<UDPawn*> GetActivePawns() const;

	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Deck")
	int32 OpenSlotCount = InitialOpenSlots;

	// ══════════════════════════════════════════════
	// B. 글로벌 카드 덱 관리
	// ══════════════════════════════════════════════

	static constexpr int32 MaxHandSize = 10;
	static constexpr int32 InitialHandSize = 5;
	static constexpr int32 DrawPerTurn = 1;

	/** 덱에 카드 추가 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	void AddCard(UDCard* Card);

	/** 스테이지 진입 시: 덱 셔플 → 풀 구성 → 초기 5장 드로우 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	void InitStage();

	/** 매 턴 시작 시: 1장 드로우 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	void StartTurnDraw();

	/** 풀에서 Count장 드로우 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	void DrawCards(int32 Count);

	/** 카드 사용: Supply/Consume 폐기, Stable → DiscardPool */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	void UseCard(UDCard* Card);

	/** 테스트 데이터 생성 (폰 2개 + 카드 6장 + 장비 11개 + 편성) */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	void InitTestData();

	/** 덱 카드 목록 조회 */
	const TArray<TObjectPtr<UDCard>>& GetDeckCards() const { return DeckCardList; }

	/** 전체 초기화 */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Deck")
	void ResetAll();

	// ── 상태 ─────────────────────────────────────
	UPROPERTY(BlueprintReadOnly, Category = "CardGame|Deck")
	TArray<TObjectPtr<UDCard>> HandCardList;

	// ── 델리게이트 ───────────────────────────────
	UPROPERTY(BlueprintAssignable, Category = "CardGame|Deck")
	FOnDeckHandChanged OnHandChanged;

private:
	// 편성 슬롯
	UPROPERTY()
	TArray<TObjectPtr<UDPawn>> PawnSlots;

	// 카드 덱
	UPROPERTY()
	TArray<TObjectPtr<UDCard>> DeckCardList;

	UPROPERTY()
	TArray<TObjectPtr<UDCard>> CardPool;

	UPROPERTY()
	TArray<TObjectPtr<UDCard>> DiscardPool;

	void ShufflePool(TArray<TObjectPtr<UDCard>>& Pool);
	void RefillFromDiscard();
};
