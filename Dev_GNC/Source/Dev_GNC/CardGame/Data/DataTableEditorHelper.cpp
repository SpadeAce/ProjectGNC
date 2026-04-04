// DataTableEditorHelper.cpp

#include "DataTableEditorHelper.h"
#include "Engine/DataTable.h"

FString UDataTableEditorHelper::GetDataTableAsJSON(UDataTable* DataTable)
{
	if (!DataTable)
	{
		return FString();
	}

	return DataTable->GetTableAsJSON();
}
