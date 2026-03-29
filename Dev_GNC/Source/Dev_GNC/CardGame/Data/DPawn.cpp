// Copyright GNC Project. All Rights Reserved.

#include "DPawn.h"
#include "DCard.h"
#include "DEquipment.h"
#include "DataSubsystem.h"
#include "TextSubsystem.h"
#include "CardGameRowTypes.h"

// ---------------------------------------------------------------------------
// 초기화
// ---------------------------------------------------------------------------

void UDPawn::InitFromId(int32 InPawnId, UDataSubsystem* DataSub, UTextSubsystem* TextSub)
{
	PawnId = InPawnId;
	DataSubRef = DataSub;

	if (DataSub)
	{
		Data = DataSub->GetPawnData(InPawnId);
		CodeName = DataSub->GetRandomName();
	}

	if (Data)
	{
		IconPath = Data->PrefabPath;
	}

	RecalculateStats();
}

void UDPawn::InitFromData(const FPawnDataRow* InData, UDataSubsystem* DataSub, UTextSubsystem* TextSub)
{
	DataSubRef = DataSub;

	if (InData)
	{
		Data = InData;
		PawnId = InData->Id;
		IconPath = InData->PrefabPath;
	}

	if (DataSub)
	{
		CodeName = DataSub->GetRandomName();
	}

	RecalculateStats();
}

// ---------------------------------------------------------------------------
// 장비
// ---------------------------------------------------------------------------

bool UDPawn::Equip(UDEquipment* Equipment)
{
	if (!Equipment || Equipment->IsEquipped()) return false;
	Equips.Add(Equipment);
	Equipment->SetEquippedPawn(InstanceId);
	RecalculateStats();
	return true;
}

bool UDPawn::Unequip(UDEquipment* Equipment)
{
	if (!Equipment) return false;
	if (Equips.Remove(Equipment) > 0)
	{
		Equipment->ClearEquippedPawn();
		RecalculateStats();
		return true;
	}
	return false;
}

// ---------------------------------------------------------------------------
// 스탯 계산
// ---------------------------------------------------------------------------

void UDPawn::RecalculateStats()
{
	if (!Data)
	{
		return;
	}

	TArray<const FEquipmentDataRow*> EquipDataList;
	for (const auto& Equip : Equips)
	{
		if (Equip && Equip->Data)
		{
			EquipDataList.Add(Equip->Data);
		}
	}

	Stats.Recalculate(*Data, EquipDataList, GrowthBonuses);
	UpdateStatus();
	OnStatsChanged.Broadcast();
}

void UDPawn::UpdateStatus()
{
	HP = Stats.Hp;
	Shield = Stats.Shield;
	Armor = Stats.Armor;
	Movement = Stats.Movement;
	Attack = Stats.Attack;
	Range = Stats.Range;
	Sight = Stats.Sight;
	Ammo = Stats.Ammo;
	Accuracy = Stats.Accuracy;
}

void UDPawn::ResetStats()
{
	Buffs.Empty();
	UpdateStatus();
	OnStatsChanged.Broadcast();
}

// ---------------------------------------------------------------------------
// 전투
// ---------------------------------------------------------------------------

void UDPawn::TakeDamage(int32 AttackPower)
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

void UDPawn::Heal(int32 Amount)
{
	if (!Data) return;
	const int32 Before = HP;
	HP = FMath::Min(HP + Amount, Stats.Hp);
	const int32 Healed = HP - Before;
	if (Healed > 0)
	{
		OnFloatingText.Broadcast(EFloatingTextType::Heal, Healed);
		OnStatsChanged.Broadcast();
	}
}

void UDPawn::AddShield(int32 Amount)
{
	Shield += Amount;
	OnFloatingText.Broadcast(EFloatingTextType::Shield, Amount);
	OnStatsChanged.Broadcast();
}

void UDPawn::ConsumeAmmo(int32 Amount)
{
	Ammo = FMath::Max(0, Ammo - Amount);
	OnStatsChanged.Broadcast();
}

void UDPawn::RestoreAmmo()
{
	Ammo = Stats.Ammo;
	OnStatsChanged.Broadcast();
}

void UDPawn::AddAmmo(int32 Amount)
{
	Ammo += Amount;
	OnStatsChanged.Broadcast();
}

// ---------------------------------------------------------------------------
// 버프
// ---------------------------------------------------------------------------

void UDPawn::ApplyBuff(ECardEffectType Type, int32 Value, int32 Duration)
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

void UDPawn::TickBuffs()
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

void UDPawn::ApplyStatDelta(ECardEffectType Type, int32 Delta)
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

// ---------------------------------------------------------------------------
// 레벨링
// ---------------------------------------------------------------------------

void UDPawn::AddExp(int32 Amount)
{
	Exp += Amount;
	TryLevelUp();
}

void UDPawn::TryLevelUp()
{
	if (!Data || !DataSubRef.IsValid())
	{
		return;
	}

	while (true)
	{
		const FPawnGrowthRow* GrowthData = DataSubRef->GetPawnGrowthByClassLevel(
			Data->ClassType, Level + 1);

		if (!GrowthData || Exp < GrowthData->RequireExp)
		{
			break;
		}

		Exp -= GrowthData->RequireExp;
		Level++;

		// 성장 보너스 적용
		const int32 BonusCount = FMath::Min(
			GrowthData->StatusType.Num(), GrowthData->StatusValue.Num());
		for (int32 i = 0; i < BonusCount; ++i)
		{
			GrowthBonuses.Add(TPair<EStatusType, int32>(
				GrowthData->StatusType[i], GrowthData->StatusValue[i]));
		}

		// 성장 카드 추가
		for (const int32 CardId : GrowthData->CardId)
		{
			GrowthCardIds.Add(CardId);
		}

		RecalculateStats();
	}
}

// ---------------------------------------------------------------------------
// 카드 핸드
// ---------------------------------------------------------------------------

void UDPawn::AddPawnHandCard(UDCard* Card)
{
	if (Card)
	{
		PawnHandCards.Add(Card);
		OnPawnHandChanged.Broadcast();
	}
}

void UDPawn::RemovePawnHandCard(UDCard* Card)
{
	if (PawnHandCards.Remove(Card) > 0)
	{
		OnPawnHandChanged.Broadcast();
	}
}

void UDPawn::BuildCardPool()
{
	PawnCardPool.Empty();
	PawnDiscardPool.Empty();
	PawnHandCards.Empty();

	if (!DataSubRef.IsValid())
	{
		return;
	}

	// 장비로부터 카드 추가
	for (const auto& Equip : Equips)
	{
		if (!Equip || !Equip->Data)
		{
			continue;
		}
		for (const int32 CardId : Equip->Data->CardId)
		{
			UDCard* Card = NewObject<UDCard>(this);
			Card->InitFromId(CardId, DataSubRef.Get());
			PawnCardPool.Add(Card);
		}
	}

	// 성장 카드 추가
	for (const int32 CardId : GrowthCardIds)
	{
		UDCard* Card = NewObject<UDCard>(this);
		Card->InitFromId(CardId, DataSubRef.Get());
		PawnCardPool.Add(Card);
	}

	ShufflePool(PawnCardPool);
}

void UDPawn::InitHand()
{
	DrawCards(InitialHandSize);
}

void UDPawn::DrawCards(int32 Count)
{
	for (int32 i = 0; i < Count; ++i)
	{
		if (PawnHandCards.Num() >= GetMaxHandSize())
		{
			break;
		}

		if (PawnCardPool.Num() == 0)
		{
			RefillFromDiscard();
			if (PawnCardPool.Num() == 0)
			{
				break;
			}
		}

		UDCard* Drawn = PawnCardPool.Pop();
		PawnHandCards.Add(Drawn);
	}

	OnPawnHandChanged.Broadcast();
}

void UDPawn::UsePawnCard(UDCard* Card)
{
	if (!Card || !Card->Data)
	{
		return;
	}

	PawnHandCards.Remove(Card);

	// Supply/Consume 타입이 아니면 버린 더미로
	if (Card->Data->Type != ECardType::Supply && Card->Data->Type != ECardType::Consume)
	{
		PawnDiscardPool.Add(Card);
	}

	OnPawnHandChanged.Broadcast();
}

void UDPawn::RefillFromDiscard()
{
	PawnCardPool.Append(PawnDiscardPool);
	PawnDiscardPool.Empty();
	ShufflePool(PawnCardPool);
}

void UDPawn::ShufflePool(TArray<TObjectPtr<UDCard>>& Pool)
{
	const int32 Num = Pool.Num();
	for (int32 i = Num - 1; i > 0; --i)
	{
		const int32 j = FMath::RandRange(0, i);
		Pool.Swap(i, j);
	}
}
