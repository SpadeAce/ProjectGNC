// Copyright GNC Project. All Rights Reserved.

#include "TextSubsystem.h"
#include "Kismet/GameplayStatics.h"

const FString UTextSubsystem::LangSaveKey = TEXT("Language");

// ---------------------------------------------------------------------------
// Initialize / Deinitialize
// ---------------------------------------------------------------------------

void UTextSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LoadSavedLanguage();
	LoadTextTable();

	UE_LOG(LogTemp, Log, TEXT("[TextSubsystem] 초기화 완료 — %d entries, Lang=%d"),
		AliasCache.Num(), static_cast<int32>(CurrentLanguage));
}

void UTextSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

// ---------------------------------------------------------------------------
// 텍스트 조회
// ---------------------------------------------------------------------------

FString UTextSubsystem::Get(const FString& Alias) const
{
	const FTextRow* const* Found = AliasCache.Find(Alias);
	if (!Found || !(*Found))
	{
		return Alias;
	}

	const FTextRow* Row = *Found;
	switch (CurrentLanguage)
	{
	case ELanguage::Kor: return Row->Kor;
	case ELanguage::Eng: return Row->Eng;
	case ELanguage::Jpn: return Row->Jpn;
	default: return Row->Kor;
	}
}

// ---------------------------------------------------------------------------
// 언어 설정
// ---------------------------------------------------------------------------

void UTextSubsystem::SetLanguage(ELanguage NewLanguage)
{
	CurrentLanguage = NewLanguage;
	SaveLanguage();
}

// ---------------------------------------------------------------------------
// 내부 구현
// ---------------------------------------------------------------------------

void UTextSubsystem::LoadTextTable()
{
	TextDataTable = LoadObject<UDataTable>(
		nullptr, TEXT("/Game/CardGame/DataTables/DT_TextData.DT_TextData"));

	if (!TextDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("[TextSubsystem] DT_TextData 로드 실패"));
		return;
	}

	const TMap<FName, uint8*>& RowMap = TextDataTable->GetRowMap();
	for (const auto& Pair : RowMap)
	{
		const FTextRow* Row = reinterpret_cast<const FTextRow*>(Pair.Value);
		if (Row && !Row->IdAlias.IsEmpty())
		{
			AliasCache.Add(Row->IdAlias, Row);
		}
	}
}

void UTextSubsystem::LoadSavedLanguage()
{
	if (GConfig)
	{
		int32 LangValue = static_cast<int32>(ELanguage::Kor);
		GConfig->GetInt(TEXT("CardGame"), *LangSaveKey, LangValue, GGameIni);
		CurrentLanguage = static_cast<ELanguage>(FMath::Clamp(LangValue, 0, 3));
	}
}

void UTextSubsystem::SaveLanguage() const
{
	if (GConfig)
	{
		GConfig->SetInt(TEXT("CardGame"), *LangSaveKey, static_cast<int32>(CurrentLanguage), GGameIni);
		GConfig->Flush(false, GGameIni);
	}
}
