// Copyright GNC Project. All Rights Reserved.

#include "PawnSubsystem.h"
#include "DPawn.h"

DEFINE_LOG_CATEGORY_STATIC(LogPawnSubsystem, Log, All);

void UPawnSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogPawnSubsystem, Log, TEXT("UPawnSubsystem Initialized"));
}

void UPawnSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UPawnSubsystem::AddPawn(UDPawn* Pawn)
{
	if (!Pawn) return;
	PawnList.Add(Pawn);
	UE_LOG(LogPawnSubsystem, Log, TEXT("AddPawn: %s (Total: %d)"), *Pawn->CodeName, PawnList.Num());
}

void UPawnSubsystem::RemovePawn(UDPawn* Pawn)
{
	if (!Pawn) return;
	PawnList.Remove(Pawn);
	UE_LOG(LogPawnSubsystem, Log, TEXT("RemovePawn: %s (Total: %d)"), *Pawn->CodeName, PawnList.Num());
}

void UPawnSubsystem::ResetAll()
{
	PawnList.Empty();
	UE_LOG(LogPawnSubsystem, Log, TEXT("ResetAll"));
}
