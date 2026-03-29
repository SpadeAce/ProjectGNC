// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CardGameTypes.generated.h"

// ============================================================
// Tile/Grid Enums (from EnumContainer.cs)
// ============================================================

UENUM(BlueprintType)
enum class ETileType : uint8
{
	Empty   = 0,
	Ground  = 1,
	Dirt    = 2,
	Water   = 3,
	Blocked = 4
};

UENUM(BlueprintType)
enum class ETileState : uint8
{
	Normal      = 0,
	Selected    = 1,
	Highlighted = 2,
	Disabled    = 3
};

UENUM(BlueprintType)
enum class ETileDirection : uint8
{
	None  = 0,
	North = 1,
	East  = 2,
	South = 3,
	West  = 4
};

UENUM(BlueprintType)
enum class ESpawnPointType : uint8
{
	None   = 0,
	Player = 1,
	Enemy  = 2
};

UENUM(BlueprintType)
enum class EEntityCategory : uint8
{
	None    = 0,
	Monster = 1,
	Pawn    = 2,
	Wall    = 3,
	Item    = 4
};

// ============================================================
// Protobuf-derived Enums (from GameData.proto)
// 정수값은 proto 정의와 일치
// ============================================================

UENUM(BlueprintType)
enum class ELanguage : uint8
{
	None = 0,
	Kor  = 1,
	Eng  = 2,
	Jpn  = 3
};

// proto 원본은 "ASSULT" 오타이나 UE5에서는 올바른 철자 사용. 정수값 2 유지.
UENUM(BlueprintType)
enum class EClassType : uint8
{
	None       = 0,
	Scout      = 1,
	Assault    = 2,
	Heavy      = 3,
	Specialist = 4
};

UENUM(BlueprintType)
enum class EItemType : uint8
{
	None      = 0,
	Card      = 1,
	Buff      = 2,
	Equipment = 3,
	Consume   = 4,
	Goods     = 5
};

UENUM(BlueprintType)
enum class ETargetType : uint8
{
	None        = 0,
	Self        = 1,
	Enemy       = 2,
	Ground      = 3,
	Ally        = 4,
	EmptyGround = 5
};

UENUM(BlueprintType)
enum class ECardType : uint8
{
	None    = 0,
	Stable  = 1,
	Supply  = 2,
	Consume = 3
};

UENUM(BlueprintType)
enum class ECardEffectType : uint8
{
	None           = 0,
	Damage         = 1,
	Heal           = 2,
	Shield         = 3,
	RestoreAction  = 4,
	Reload         = 5,
	DrawCard       = 6,
	BuffAttack     = 7,
	BuffArmor      = 8,
	BuffMovement   = 9,
	DebuffAttack   = 10,
	DebuffArmor    = 11,
	DebuffMovement = 12
};

UENUM(BlueprintType)
enum class ETileEntityType : uint8
{
	None     = 0,
	Actor    = 1,
	WallHalf = 2,
	WallFull = 3
};

UENUM(BlueprintType)
enum class EStatusType : uint8
{
	None      = 0,
	Atk       = 1,
	Def       = 2,
	HP        = 3,
	Shield    = 4,
	Movement  = 5,
	Range     = 6,
	Accuracy  = 7,
	CardCap   = 8,
	AmmoCap   = 9,
	EnergyCap = 10
};

UENUM(BlueprintType)
enum class EEquipSlotType : uint8
{
	None       = 0,
	WeaponMain = 1,
	WeaponSub  = 2,
	Armor      = 3,
	Helmet     = 4,
	Tool       = 5
};

// ============================================================
// UI/Feedback Enum
// ============================================================

UENUM(BlueprintType)
enum class EFloatingTextType : uint8
{
	Damage = 0,
	Heal   = 1,
	Shield = 2,
	Buff   = 3,
	Debuff = 4,
	Block  = 5,
	Miss   = 6
};

// ============================================================
// Turn System Enum
// ============================================================

UENUM(BlueprintType)
enum class ETurnPhase : uint8
{
	Pawn    = 0,
	Monster = 1
};
