// Copyright GNC Project. All Rights Reserved.

#include "DMonster.h"
#include "DataSubsystem.h"
#include "CardGameRowTypes.h"

// ---------------------------------------------------------------------------
// 초기화
// ---------------------------------------------------------------------------

void UDMonster::InitFromId(int32 InMonsterId, UDataSubsystem* DataSub)
{
	MonsterId = InMonsterId;
	if (DataSub)
	{
		Data = DataSub->GetMonsterData(InMonsterId);
	}
	UpdateStatus();
}

void UDMonster::InitFromData(const FMonsterDataRow* InData)
{
	if (InData)
	{
		Data = InData;
		MonsterId = InData->Id;
	}
	UpdateStatus();
}

void UDMonster::UpdateStatus()
{
	if (!Data) return;

	HP = Data->HP;
	Shield = Data->Shield;
	Armor = Data->Armor;
	Movement = Data->Movement;
	Attack = Data->Attack;
	Range = Data->Range;
	Sight = Data->Sight;
}

// ---------------------------------------------------------------------------
// 전투
// ---------------------------------------------------------------------------

void UDMonster::TakeDamage(int32 AttackPower)
{
	int32 Damage = FMath::Max(0, AttackPower - Armor);

	if (Damage > 0 && Shield > 0)
	{
		const int32 ShieldAbsorb = FMath::Min(Shield, Damage);
		Shield -= ShieldAbsorb;
		Damage -= ShieldAbsorb;
		if (ShieldAbsorb > 0)
		{
			OnFloatingText.Broadcast(EFloatingTextType::Shield, -ShieldAbsorb);
		}
	}

	if (Damage > 0)
	{
		HP -= Damage;
		OnFloatingText.Broadcast(EFloatingTextType::Damage, -Damage);
	}
	else if (AttackPower > 0)
	{
		OnFloatingText.Broadcast(EFloatingTextType::Block, 0);
	}

	OnStatsChanged.Broadcast();
}

void UDMonster::Heal(int32 Amount)
{
	if (!Data) return;
	const int32 Before = HP;
	HP = FMath::Min(HP + Amount, Data->HP);
	const int32 Healed = HP - Before;
	if (Healed > 0)
	{
		OnFloatingText.Broadcast(EFloatingTextType::Heal, Healed);
		OnStatsChanged.Broadcast();
	}
}

void UDMonster::AddShield(int32 Amount)
{
	Shield += Amount;
	OnFloatingText.Broadcast(EFloatingTextType::Shield, Amount);
	OnStatsChanged.Broadcast();
}

void UDMonster::NotifyMiss()
{
	OnFloatingText.Broadcast(EFloatingTextType::Miss, 0);
}

// ---------------------------------------------------------------------------
// 버프
// ---------------------------------------------------------------------------

void UDMonster::ApplyBuff(ECardEffectType Type, int32 Value, int32 Duration)
{
	Buffs.Add(FBuffEntry(Type, Value, Duration));
	ApplyStatDelta(Type, Value);

	const bool bIsDebuff = (Type == ECardEffectType::DebuffAttack
		|| Type == ECardEffectType::DebuffArmor
		|| Type == ECardEffectType::DebuffMovement);
	OnFloatingText.Broadcast(
		bIsDebuff ? EFloatingTextType::Debuff : EFloatingTextType::Buff, Value);
	OnStatsChanged.Broadcast();
}

void UDMonster::TickBuffs()
{
	bool bChanged = false;
	for (int32 i = Buffs.Num() - 1; i >= 0; --i)
	{
		Buffs[i].RemainingTurns--;
		if (Buffs[i].RemainingTurns <= 0)
		{
			ApplyStatDelta(Buffs[i].Type, -Buffs[i].Value);
			Buffs.RemoveAt(i);
			bChanged = true;
		}
	}
	if (bChanged)
	{
		OnStatsChanged.Broadcast();
	}
}

void UDMonster::ApplyStatDelta(ECardEffectType Type, int32 Delta)
{
	switch (Type)
	{
	case ECardEffectType::BuffAttack:
	case ECardEffectType::DebuffAttack:
		Attack += Delta;
		break;
	case ECardEffectType::BuffArmor:
	case ECardEffectType::DebuffArmor:
		Armor += Delta;
		break;
	case ECardEffectType::BuffMovement:
	case ECardEffectType::DebuffMovement:
		Movement += Delta;
		break;
	default:
		break;
	}
}
