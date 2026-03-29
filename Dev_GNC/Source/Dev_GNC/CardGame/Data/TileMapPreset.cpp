// Copyright GNC Project. All Rights Reserved.

#include "TileMapPreset.h"

const FTilePresetData* UTileMapPreset::GetTileData(FIntPoint Position) const
{
	for (const FTilePresetData& Data : Tiles)
	{
		if (Data.Position == Position)
		{
			return &Data;
		}
	}
	return nullptr;
}
