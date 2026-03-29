// Copyright GNC Project. All Rights Reserved.

#include "DCard.h"
#include "DataSubsystem.h"

void UDCard::InitFromId(int32 InCardId, UDataSubsystem* DataSub)
{
	CardId = InCardId;
	ItemId = InCardId;
	if (DataSub)
	{
		Data = DataSub->GetCardData(InCardId);
	}
}

void UDCard::InitFromData(const FCardDataRow* InData)
{
	if (InData)
	{
		CardId = InData->Id;
		ItemId = InData->Id;
		Data = InData;
	}
}
