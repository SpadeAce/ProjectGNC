// Copyright GNC Project. All Rights Reserved.

#include "HUDActorWidget.h"
#include "DPawn.h"
#include "DMonster.h"
#include "CardGameRowTypes.h"
#include "TextSubsystem.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UHUDActorWidget::InitFromPawn(UDPawn* Pawn)
{
	if (!Pawn) return;

	BoundPawn = Pawn;
	BoundMonster = nullptr;
	MaxHP = Pawn->Data ? Pawn->Data->HP : FMath::Max(1, Pawn->HP);

	Pawn->OnStatsChanged.AddDynamic(this, &UHUDActorWidget::Refresh);

	if (TextAmmo)
	{
		TextAmmo->SetVisibility(ESlateVisibility::HitTestInvisible);
	}

	Refresh();
}

void UHUDActorWidget::InitFromMonster(UDMonster* Monster)
{
	if (!Monster) return;

	BoundMonster = Monster;
	BoundPawn = nullptr;
	MaxHP = Monster->Data ? Monster->Data->HP : FMath::Max(1, Monster->HP);

	Monster->OnStatsChanged.AddDynamic(this, &UHUDActorWidget::Refresh);

	// 몬스터에는 탄약 표시 없음
	if (TextAmmo)
	{
		TextAmmo->SetVisibility(ESlateVisibility::Collapsed);
	}

	Refresh();
}

void UHUDActorWidget::Refresh()
{
	int32 HP = 0;
	int32 Shield = 0;
	int32 Armor = 0;
	int32 Ammo = 0;
	FString Name;

	UTextSubsystem* TextSub = GetGameInstance() ? GetGameInstance()->GetSubsystem<UTextSubsystem>() : nullptr;

	if (UDPawn* Pawn = BoundPawn.Get())
	{
		HP = Pawn->HP;
		Shield = Pawn->Shield;
		Armor = Pawn->Armor;
		Ammo = Pawn->Ammo;
		Name = TextSub ? TextSub->Get(Pawn->CodeName) : Pawn->CodeName;
	}
	else if (UDMonster* Monster = BoundMonster.Get())
	{
		HP = Monster->HP;
		Shield = Monster->Shield;
		Armor = Monster->Armor;
		if (Monster->Data)
		{
			Name = TextSub ? TextSub->Get(Monster->Data->NameAlias) : Monster->Data->NameAlias;
		}
		else
		{
			Name = TEXT("???");
		}
	}

	// HP 바
	if (BarHP)
	{
		const float HPPercent = MaxHP > 0 ? FMath::Clamp((float)HP / MaxHP, 0.f, 1.f) : 0.f;
		BarHP->SetPercent(HPPercent);
	}

	// Shield 바
	if (BarShield)
	{
		if (Shield > 0)
		{
			BarShield->SetVisibility(ESlateVisibility::HitTestInvisible);
			const float ShieldPercent = MaxHP > 0 ? FMath::Min(1.f, (float)Shield / MaxHP) : 0.f;
			BarShield->SetPercent(ShieldPercent);
		}
		else
		{
			BarShield->SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// Armor 텍스트
	if (TextArmor)
	{
		TextArmor->SetText(FText::AsNumber(Armor));
	}

	// Ammo 텍스트 (폰만)
	if (TextAmmo && BoundPawn.IsValid())
	{
		TextAmmo->SetText(FText::AsNumber(Ammo));
	}

	// 이름
	if (TextName)
	{
		TextName->SetText(FText::FromString(Name));
	}
}

void UHUDActorWidget::NativeDestruct()
{
	if (UDPawn* Pawn = BoundPawn.Get())
	{
		Pawn->OnStatsChanged.RemoveDynamic(this, &UHUDActorWidget::Refresh);
	}
	if (UDMonster* Monster = BoundMonster.Get())
	{
		Monster->OnStatsChanged.RemoveDynamic(this, &UHUDActorWidget::Refresh);
	}

	Super::NativeDestruct();
}
