// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CardGameTypes.h"
#include "EntityCatalog.generated.h"

USTRUCT(BlueprintType)
struct FEntityCatalogEntry
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 Id = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) EEntityCategory Category = EEntityCategory::None;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftClassPtr<AActor> ActorClass;
};

/**
 * UEntityCatalog
 * 엔티티 프리팹 카탈로그 데이터 에셋.
 * Unity EntityCatalog (ScriptableObject) 대응.
 */
UCLASS(BlueprintType)
class DEV_GNC_API UEntityCatalog : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CardGame")
	TArray<FEntityCatalogEntry> Entries;

	const FEntityCatalogEntry* GetEntry(int32 Id) const;

private:
	mutable TMap<int32, int32> LookupCache;
	mutable bool bCacheBuilt = false;

	void BuildCache() const;
};
