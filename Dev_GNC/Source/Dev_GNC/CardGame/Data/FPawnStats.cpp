// Copyright GNC Project. All Rights Reserved.

#include "FPawnStats.h"
#include "CardGameRowTypes.h"

void FPawnStats::Recalculate(
	const FPawnDataRow& BaseData,
	const TArray<const FEquipmentDataRow*>& Equips,
	const TArray<TPair<EStatusType, int32>>& GrowthBonuses)
{
	// 기본 스탯 복사
	Hp = BaseData.HP;
	Attack = BaseData.Attack;
	Armor = BaseData.Armor;
	Shield = BaseData.Shield;
	Movement = BaseData.Movement;
	Range = BaseData.Range;
	Sight = BaseData.Sight;
	Ammo = BaseData.Ammo;
	Accuracy = BaseData.Accuracy;
	CardCap = 0;

	// 장비 보너스 적용
	for (const FEquipmentDataRow* Equip : Equips)
	{
		if (!Equip) continue;

		const int32 BonusCount = FMath::Min(
			Equip->StatusType.Num(), Equip->StatusValue.Num());
		for (int32 i = 0; i < BonusCount; ++i)
		{
			ApplyStatusBonus(Equip->StatusType[i], Equip->StatusValue[i]);
		}
	}

	// 성장 보너스 적용
	for (const auto& Pair : GrowthBonuses)
	{
		ApplyStatusBonus(Pair.Key, Pair.Value);
	}
}

void FPawnStats::ApplyStatusBonus(EStatusType Type, int32 Value)
{
	switch (Type)
	{
	case EStatusType::Atk:       Attack += Value; break;
	case EStatusType::Def:       Armor += Value; break;
	case EStatusType::HP:        Hp += Value; break;
	case EStatusType::Shield:    Shield += Value; break;
	case EStatusType::Movement:  Movement += Value; break;
	case EStatusType::Range:     Range += Value; break;
	case EStatusType::Accuracy:  Accuracy += Value; break;
	case EStatusType::CardCap:   CardCap += Value; break;
	case EStatusType::AmmoCap:   Ammo += Value; break;
	case EStatusType::EnergyCap: break; // 에너지는 턴 시스템에서 관리
	default: break;
	}
}
