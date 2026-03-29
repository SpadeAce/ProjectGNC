// Copyright GNC Project. All Rights Reserved.

#include "EntityCatalog.h"

const FEntityCatalogEntry* UEntityCatalog::GetEntry(int32 Id) const
{
	if (Id <= 0) return nullptr;

	if (!bCacheBuilt)
	{
		BuildCache();
	}

	const int32* IndexPtr = LookupCache.Find(Id);
	if (IndexPtr == nullptr) return nullptr;

	return &Entries[*IndexPtr];
}

void UEntityCatalog::BuildCache() const
{
	LookupCache.Empty();
	for (int32 i = 0; i < Entries.Num(); ++i)
	{
		if (Entries[i].Id > 0)
		{
			LookupCache.Add(Entries[i].Id, i);
		}
	}
	bCacheBuilt = true;
}
