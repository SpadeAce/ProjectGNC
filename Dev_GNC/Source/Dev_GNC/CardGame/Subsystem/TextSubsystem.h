// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CardGameTypes.h"
#include "CardGameRowTypes.h"
#include "TextSubsystem.generated.h"

/**
 * UTextSubsystem
 * 로컬라이제이션 텍스트 조회.
 * Unity TextManager.Instance.Get(alias) 패턴 대체.
 */
UCLASS()
class UTextSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	/** alias에 해당하는 현재 언어 텍스트 반환. 없으면 alias 그대로 반환. */
	UFUNCTION(BlueprintCallable, Category = "CardGame|Text")
	FString Get(const FString& Alias) const;

	UFUNCTION(BlueprintCallable, Category = "CardGame|Text")
	ELanguage GetCurrentLanguage() const { return CurrentLanguage; }

	UFUNCTION(BlueprintCallable, Category = "CardGame|Text")
	void SetLanguage(ELanguage NewLanguage);

private:
	static const FString LangSaveKey;

	UPROPERTY()
	TObjectPtr<UDataTable> TextDataTable;

	ELanguage CurrentLanguage = ELanguage::Kor;

	TMap<FString, const FTextRow*> AliasCache;

	void LoadTextTable();
	void LoadSavedLanguage();
	void SaveLanguage() const;
};
