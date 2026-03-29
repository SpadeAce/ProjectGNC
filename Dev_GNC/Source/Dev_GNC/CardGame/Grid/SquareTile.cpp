// Copyright GNC Project. All Rights Reserved.

#include "SquareTile.h"
#include "GridSlotComponent.h"
#include "Components/StaticMeshComponent.h"

ASquareTile::ASquareTile()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Slot = CreateDefaultSubobject<UGridSlotComponent>(TEXT("Slot"));

	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(SceneRoot);

	SelectedOverlay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SelectedOverlay"));
	SelectedOverlay->SetupAttachment(SceneRoot);
	SelectedOverlay->SetVisibility(false);

	HighlightedOverlay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighlightedOverlay"));
	HighlightedOverlay->SetupAttachment(SceneRoot);
	HighlightedOverlay->SetVisibility(false);

	TargetOverlay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TargetOverlay"));
	TargetOverlay->SetupAttachment(SceneRoot);
	TargetOverlay->SetVisibility(false);

	RadiusOverlay = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RadiusOverlay"));
	RadiusOverlay->SetupAttachment(SceneRoot);
	RadiusOverlay->SetVisibility(false);
}

void ASquareTile::Init(int32 InTileId, FIntPoint InGridPosition, ETileType InType)
{
	TileId = InTileId;
	GridPosition = InGridPosition;
	TileType = InType;
	ApplyTileTypeMesh();
	SetState(ETileState::Normal);
}

bool ASquareTile::IsWalkable() const
{
	return TileType != ETileType::Blocked && TileType != ETileType::Water;
}

void ASquareTile::SetTileType(ETileType InType)
{
	TileType = InType;
	ApplyTileTypeMesh();
}

void ASquareTile::SetState(ETileState NewState)
{
	if (TileState == NewState) return;

	TileState = NewState;
	UpdateVisuals();
	OnStateChanged.Broadcast(this, NewState);
}

void ASquareTile::ToggleSelection()
{
	SetState(TileState == ETileState::Selected ? ETileState::Normal : ETileState::Selected);
	OnTileClicked.Broadcast(this);
}

void ASquareTile::SetSelected(bool bSelected)
{
	SetState(bSelected ? ETileState::Selected : ETileState::Normal);
}

void ASquareTile::SetHighlighted(bool bHighlighted)
{
	if (TileState == ETileState::Selected) return;
	SetState(bHighlighted ? ETileState::Highlighted : ETileState::Normal);
}

void ASquareTile::SetTarget(bool bTarget)
{
	if (TargetOverlay)
	{
		TargetOverlay->SetVisibility(bTarget);
	}
}

void ASquareTile::SetRadius(bool bActive)
{
	if (RadiusOverlay)
	{
		RadiusOverlay->SetVisibility(bActive);
	}
}

void ASquareTile::Clear()
{
	SetState(ETileState::Normal);
	SetRadius(false);
	OnStateChanged.Clear();
	OnTileClicked.Clear();
}

void ASquareTile::ApplyTileTypeMesh()
{
	if (!TileMesh) return;

	const TSoftObjectPtr<UStaticMesh>* Found = TileMeshByType.Find(TileType);
	if (Found && !Found->IsNull())
	{
		UStaticMesh* Mesh = Found->LoadSynchronous();
		TileMesh->SetStaticMesh(Mesh);
		TileMesh->EmptyOverrideMaterials();
	}
}

void ASquareTile::UpdateVisuals()
{
	if (SelectedOverlay)
	{
		SelectedOverlay->SetVisibility(TileState == ETileState::Selected);
	}
	if (HighlightedOverlay)
	{
		HighlightedOverlay->SetVisibility(TileState == ETileState::Highlighted);
	}
}
