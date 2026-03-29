// Copyright GNC Project. All Rights Reserved.

#include "CardStageGameMode.h"
#include "CardGameCameraPawn.h"
#include "Data/TileMapPreset.h"
#include "Data/CardGameRowTypes.h"
#include "Subsystem/DataSubsystem.h"
#include "Subsystem/PlayerDataSubsystem.h"
#include "Subsystem/StageSubsystem.h"
#include "Subsystem/GridSubsystem.h"

DEFINE_LOG_CATEGORY_STATIC(LogCardStageGM, Log, All);

void ACardStageGameMode::BeginPlay()
{
	Super::BeginPlay();

	LoadStageFromPreset();
	SetCameraPivot();
}

void ACardStageGameMode::LoadStageFromPreset()
{
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	UDataSubsystem* DataSub = GI->GetSubsystem<UDataSubsystem>();
	UPlayerDataSubsystem* PlayerSub = GI->GetSubsystem<UPlayerDataSubsystem>();
	if (!DataSub || !PlayerSub) return;

	// 난이도에 맞는 스테이지 프리셋 조회
	const FStagePresetRow* PresetRow = DataSub->GetStagePresetByLevel(PlayerSub->DifficultyLevel);
	if (!PresetRow || PresetRow->PresetPath.Num() == 0)
	{
		UE_LOG(LogCardStageGM, Error, TEXT("No stage preset found for difficulty %d"), PlayerSub->DifficultyLevel);
		return;
	}

	// 프리셋 경로 배열에서 랜덤 선택
	const int32 RandIdx = FMath::RandRange(0, PresetRow->PresetPath.Num() - 1);
	const TSoftObjectPtr<UTileMapPreset>& PresetRef = PresetRow->PresetPath[RandIdx];

	// DataAsset 로드
	UTileMapPreset* Preset = PresetRef.LoadSynchronous();
	if (!Preset)
	{
		UE_LOG(LogCardStageGM, Error, TEXT("Failed to load TileMapPreset: %s"), *PresetRef.ToString());
		return;
	}

	// StageSubsystem에 스테이지 로드 (내부에서 InitStage, StartGame 체인 완료)
	UStageSubsystem* StageSub = GetWorld()->GetSubsystem<UStageSubsystem>();
	if (StageSub)
	{
		StageSub->LoadStage(Preset);
		UE_LOG(LogCardStageGM, Log, TEXT("Stage loaded: Preset=%s, Difficulty=%d"), *PresetRef.ToString(), PlayerSub->DifficultyLevel);
	}
}

void ACardStageGameMode::SetCameraPivot()
{
	UGridSubsystem* GridSub = GetWorld()->GetSubsystem<UGridSubsystem>();
	if (!GridSub) return;

	FVector MapCenter = GridSub->GetMapCenter();

	// 카메라 폰의 피벗 설정
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (PC)
	{
		if (ACardGameCameraPawn* CamPawn = Cast<ACardGameCameraPawn>(PC->GetPawn()))
		{
			CamPawn->SetPivot(MapCenter);
		}
	}

	UE_LOG(LogCardStageGM, Log, TEXT("Camera pivot set to map center: %s"), *MapCenter.ToString());
}
