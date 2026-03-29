// Copyright GNC Project. All Rights Reserved.

#include "DEquipment.h"
#include "DataSubsystem.h"

void UDEquipment::InitFromId(int32 InEquipId, UDataSubsystem* DataSub)
{
	EquipmentId = InEquipId;
	if (DataSub)
	{
		Data = DataSub->GetEquipmentData(InEquipId);
	}
}

void UDEquipment::InitFromData(const FEquipmentDataRow* InData)
{
	if (InData)
	{
		EquipmentId = InData->Id;
		Data = InData;
	}
}
