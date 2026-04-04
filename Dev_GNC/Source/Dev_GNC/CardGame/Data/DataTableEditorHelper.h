// DataTableEditorHelper.h
// UDataTable::GetTableAsJSON()을 Python에서 호출 가능하도록 UFUNCTION으로 래핑

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataTableEditorHelper.generated.h"

UCLASS()
class UDataTableEditorHelper : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** DataTable의 전체 내용을 JSON 문자열로 반환한다. */
	UFUNCTION(BlueprintCallable, Category = "DataTable|Editor")
	static FString GetDataTableAsJSON(UDataTable* DataTable);
};
