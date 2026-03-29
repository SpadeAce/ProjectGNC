// Copyright GNC Project. All Rights Reserved.

#include "PlayerDataSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogPlayerDataSubsystem, Log, All);

// ────────────────────────────────────────────────────────────
// Lifecycle
// ────────────────────────────────────────────────────────────

void UPlayerDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogPlayerDataSubsystem, Log, TEXT("UPlayerDataSubsystem Initialized (Gold=%d, Difficulty=%d)"), Gold, DifficultyLevel);
}

void UPlayerDataSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ────────────────────────────────────────────────────────────
// 골드
// ────────────────────────────────────────────────────────────

void UPlayerDataSubsystem::AddGold(int32 Amount)
{
	Gold += Amount;
	UE_LOG(LogPlayerDataSubsystem, Log, TEXT("AddGold: +%d → %d"), Amount, Gold);
}

bool UPlayerDataSubsystem::SpendGold(int32 Amount)
{
	if (Gold < Amount) return false;
	Gold -= Amount;
	UE_LOG(LogPlayerDataSubsystem, Log, TEXT("SpendGold: -%d → %d"), Amount, Gold);
	return true;
}

bool UPlayerDataSubsystem::HasEnoughGold(int32 Amount) const
{
	return Gold >= Amount;
}

// ────────────────────────────────────────────────────────────
// 난이도
// ────────────────────────────────────────────────────────────

void UPlayerDataSubsystem::IncrementDifficulty()
{
	DifficultyLevel++;
	UE_LOG(LogPlayerDataSubsystem, Log, TEXT("IncrementDifficulty → %d"), DifficultyLevel);
}

// ────────────────────────────────────────────────────────────
// 리셋
// ────────────────────────────────────────────────────────────

void UPlayerDataSubsystem::ResetAll()
{
	Gold = 1000;
	DifficultyLevel = 1;
	UE_LOG(LogPlayerDataSubsystem, Log, TEXT("ResetAll"));
}
