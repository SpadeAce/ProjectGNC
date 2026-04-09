// Copyright GNC Project. All Rights Reserved.

#include "GridSubsystem.h"
#include "SquareTile.h"
#include "EdgeTile.h"
#include "GridEntity.h"
#include "GridSlotComponent.h"
#include "TileWall.h"
#include "TileMapPreset.h"
#include "EntityCatalog.h"
#include "DObject.h"

// ════════════════════════════════════════════
// 방향 상수
// ════════════════════════════════════════════

static const TArray<FIntPoint> GSquareDirections = {
	FIntPoint(0, 1),   // North (+Y)
	FIntPoint(1, 0),   // East  (+X)
	FIntPoint(0, -1),  // South (-Y)
	FIntPoint(-1, 0)   // West  (-X)
};

static const TMap<ETileDirection, FIntPoint> GDirectionToOffset = {
	{ ETileDirection::North, FIntPoint(0, 1) },
	{ ETileDirection::East,  FIntPoint(1, 0) },
	{ ETileDirection::South, FIntPoint(0, -1) },
	{ ETileDirection::West,  FIntPoint(-1, 0) }
};

static const TMap<FIntPoint, ETileDirection> GOffsetToDirection = {
	{ FIntPoint(0, 1),  ETileDirection::North },
	{ FIntPoint(1, 0),  ETileDirection::East },
	{ FIntPoint(0, -1), ETileDirection::South },
	{ FIntPoint(-1, 0), ETileDirection::West }
};

const TArray<FIntPoint>& UGridSubsystem::GetSquareDirections()
{
	return GSquareDirections;
}

FIntPoint UGridSubsystem::DirectionToOffset(ETileDirection Dir)
{
	const FIntPoint* Found = GDirectionToOffset.Find(Dir);
	return Found ? *Found : FIntPoint::ZeroValue;
}

ETileDirection UGridSubsystem::OffsetToDirection(FIntPoint Offset)
{
	const ETileDirection* Found = GOffsetToDirection.Find(Offset);
	return Found ? *Found : ETileDirection::None;
}

// ════════════════════════════════════════════
// 라이프사이클
// ════════════════════════════════════════════

void UGridSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	SquareTileClass = LoadClass<ASquareTile>(
		nullptr,
		TEXT("/Game/CardGame/Blueprints/Grid/BP_SquareTile.BP_SquareTile_C"));

	if (!SquareTileClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[GridSubsystem] BP_SquareTile 로드 실패 — C++ 클래스로 fallback"));
		SquareTileClass = ASquareTile::StaticClass();
	}
}

void UGridSubsystem::Deinitialize()
{
	ClearAllTiles();
	Super::Deinitialize();
}

// ════════════════════════════════════════════
// 맵 생성
// ════════════════════════════════════════════

bool UGridSubsystem::GenerateTileMap(int32 Width, int32 Height, float InTileSize)
{
	if (Width <= 0 || Height <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[GridSubsystem] Width and height must be greater than 0"));
		return false;
	}

	ClearAllTiles();
	TileSize = InTileSize;
	NextTileId = 0;

	for (int32 X = 0; X < Width; ++X)
	{
		for (int32 Y = 0; Y < Height; ++Y)
		{
			CreateTile(FIntPoint(X, Y));
		}
	}

	GenerateEdgeTiles();

	UE_LOG(LogTemp, Log, TEXT("[GridSubsystem] Generated %d tiles, %d edges (%dx%d, tileSize: %.2f)"),
		TilesByPosition.Num(), EdgeTiles.Num(), Width, Height, TileSize);
	return true;
}

bool UGridSubsystem::GenerateTileMapFromPreset(UTileMapPreset* Preset)
{
	if (Preset == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("[GridSubsystem] Preset is null"));
		return false;
	}

	if (Preset->Width <= 0 || Preset->Height <= 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[GridSubsystem] Preset width and height must be greater than 0"));
		return false;
	}

	ClearAllTiles();
	TileSize = Preset->TileSize;
	NextTileId = 0;

	// 타일 생성
	for (int32 X = 0; X < Preset->Width; ++X)
	{
		for (int32 Y = 0; Y < Preset->Height; ++Y)
		{
			FIntPoint GridPos(X, Y);
			const FTilePresetData* PresetData = Preset->GetTileData(GridPos);
			ETileType Type = PresetData ? PresetData->TileType : ETileType::Ground;

			if (Type == ETileType::Empty)
				continue;

			CreateTile(GridPos, Type);
		}
	}

	GenerateEdgeTiles();

	// 타일 엔티티 배치
	for (const FTilePresetData& TileData : Preset->Tiles)
	{
		if (TileData.EntityId <= 0) continue;

		ASquareTile* Tile = GetTile(TileData.Position);
		if (Tile == nullptr) continue;

		AActor* EntityActor = SpawnEntityFromCatalog(TileData.EntityId);
		if (EntityActor == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GridSubsystem] Entity not found in catalog: id=%d"), TileData.EntityId);
			continue;
		}

		AGridEntity* Entity = Cast<AGridEntity>(EntityActor);
		if (Entity && Tile->Slot)
		{
			Tile->Slot->SetEntity(Entity);
		}
		else
		{
			EntityActor->Destroy();
		}
	}

	// 엣지 엔티티 배치 (Wall 등)
	for (const FEdgePresetData& EdgeData : Preset->Edges)
	{
		if (EdgeData.EntityId <= 0) continue;

		FEdgeTileKey Key(EdgeData.PosA, EdgeData.PosB);
		TObjectPtr<AEdgeTile>* EdgePtr = EdgeTiles.Find(Key);
		if (EdgePtr == nullptr) continue;
		AEdgeTile* Edge = *EdgePtr;

		AActor* EntityActor = SpawnEntityFromCatalog(EdgeData.EntityId);
		if (EntityActor == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("[GridSubsystem] Edge entity not found in catalog: id=%d"), EdgeData.EntityId);
			continue;
		}

		AGridEntity* Entity = Cast<AGridEntity>(EntityActor);
		if (Entity && Edge->Slot)
		{
			Edge->Slot->SetEntity(Entity);
		}
		else
		{
			EntityActor->Destroy();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("[GridSubsystem] Generated tilemap from preset: %d tiles (%dx%d)"),
		TilesByPosition.Num(), Preset->Width, Preset->Height);
	return true;
}

void UGridSubsystem::ClearAllTiles()
{
	// EdgeTile 먼저 정리
	for (auto& Pair : EdgeTiles)
	{
		if (Pair.Value)
		{
			Pair.Value->Clear();
			Pair.Value->Destroy();
		}
	}
	EdgeTiles.Empty();

	// SquareTile 정리
	for (auto& Pair : TilesByPosition)
	{
		if (Pair.Value)
		{
			Pair.Value->Clear();
			Pair.Value->Destroy();
		}
	}
	TilesByPosition.Empty();
	TilesById.Empty();
	NextTileId = 0;
	CachedEntityCatalog = nullptr;
}

// ════════════════════════════════════════════
// 타일 조회
// ════════════════════════════════════════════

ASquareTile* UGridSubsystem::GetTile(FIntPoint GridPosition) const
{
	const TObjectPtr<ASquareTile>* Found = TilesByPosition.Find(GridPosition);
	return Found ? *Found : nullptr;
}

ASquareTile* UGridSubsystem::GetTileById(int32 Id) const
{
	const TObjectPtr<ASquareTile>* Found = TilesById.Find(Id);
	return Found ? *Found : nullptr;
}

TArray<ASquareTile*> UGridSubsystem::GetAdjacentTiles(FIntPoint GridPosition) const
{
	TArray<ASquareTile*> Result;
	Result.Reserve(4);

	for (const FIntPoint& Dir : GSquareDirections)
	{
		FIntPoint NeighborPos = GridPosition + Dir;
		ASquareTile* Neighbor = GetTile(NeighborPos);
		if (Neighbor)
		{
			Result.Add(Neighbor);
		}
	}

	return Result;
}

TArray<ASquareTile*> UGridSubsystem::GetTilesInRange(FIntPoint Center, int32 Range) const
{
	TArray<ASquareTile*> Result;

	for (int32 X = -Range; X <= Range; ++X)
	{
		for (int32 Y = -Range; Y <= Range; ++Y)
		{
			ASquareTile* Tile = GetTile(Center + FIntPoint(X, Y));
			if (Tile)
			{
				Result.Add(Tile);
			}
		}
	}

	return Result;
}

// ════════════════════════════════════════════
// EdgeTile 조회
// ════════════════════════════════════════════

AEdgeTile* UGridSubsystem::GetEdgeTile(FIntPoint GridPos, ETileDirection Direction) const
{
	if (Direction == ETileDirection::None) return nullptr;

	FIntPoint Offset = DirectionToOffset(Direction);
	FIntPoint NeighborPos = GridPos + Offset;
	FEdgeTileKey Key(GridPos, NeighborPos);

	const TObjectPtr<AEdgeTile>* Found = EdgeTiles.Find(Key);
	return Found ? *Found : nullptr;
}

TArray<AEdgeTile*> UGridSubsystem::GetEdgeTilesForTile(FIntPoint GridPosition) const
{
	TArray<AEdgeTile*> Result;
	Result.Reserve(4);

	for (const FIntPoint& Dir : GSquareDirections)
	{
		FEdgeTileKey Key(GridPosition, GridPosition + Dir);
		const TObjectPtr<AEdgeTile>* Found = EdgeTiles.Find(Key);
		if (Found)
		{
			Result.Add(*Found);
		}
	}

	return Result;
}

// ════════════════════════════════════════════
// 경로 탐색
// ════════════════════════════════════════════

TArray<ASquareTile*> UGridSubsystem::GetReachableTiles(FIntPoint Center, int32 Movement, UClass* FriendlyDataClass)
{
	TArray<ASquareTile*> Reachable;
	TMap<FIntPoint, int32> Visited; // pos → 남은 이동력
	TQueue<TPair<FIntPoint, int32>> Queue;

	Visited.Add(Center, Movement);
	Queue.Enqueue(TPair<FIntPoint, int32>(Center, Movement));

	while (!Queue.IsEmpty())
	{
		TPair<FIntPoint, int32> Current;
		Queue.Dequeue(Current);
		FIntPoint Pos = Current.Key;
		int32 Remaining = Current.Value;

		for (const FIntPoint& Dir : GSquareDirections)
		{
			FIntPoint Next = Pos + Dir;
			ASquareTile* Tile = GetTile(Next);
			if (Tile == nullptr) continue;
			if (!Tile->IsWalkable()) continue;
			if (!IsEdgePassable(Pos, Next)) continue;

			int32 NextRemaining = Remaining - 1;
			int32* PrevRemaining = Visited.Find(Next);
			if (PrevRemaining && *PrevRemaining >= NextRemaining) continue;
			Visited.Add(Next, NextRemaining);

			AGridEntity* Occupant = (Tile->Slot) ? Tile->Slot->SlotEntity : nullptr;
			if (Occupant != nullptr)
			{
				// 우군 통과: reachable에 추가하지 않고 BFS만 계속
				bool bIsFriendly = FriendlyDataClass != nullptr
					&& Occupant->Data != nullptr
					&& Occupant->Data->IsA(FriendlyDataClass);

				if (!bIsFriendly) continue; // 다른 종류 → 차단
			}
			else
			{
				Reachable.Add(Tile);
			}

			if (NextRemaining > 0)
			{
				Queue.Enqueue(TPair<FIntPoint, int32>(Next, NextRemaining));
			}
		}
	}

	return Reachable;
}

TArray<ASquareTile*> UGridSubsystem::GetPath(FIntPoint From, FIntPoint To, int32 Movement, UClass* FriendlyDataClass)
{
	TArray<ASquareTile*> EmptyResult;

	// A* 노드
	struct FPathNode
	{
		FIntPoint Pos;
		int32 G; // 이동 횟수
		int32 F; // G + H

		bool operator>(const FPathNode& Other) const { return F > Other.F; }
	};

	auto Heuristic = [&To](FIntPoint P) -> int32
	{
		return FMath::Abs(P.X - To.X) + FMath::Abs(P.Y - To.Y);
	};

	TMap<FIntPoint, FIntPoint> Predecessors;
	TMap<FIntPoint, int32> BestG; // pos → 최소 이동 횟수

	// min-heap
	TArray<FPathNode> OpenHeap;
	auto HeapPush = [&OpenHeap](const FPathNode& Node)
	{
		OpenHeap.HeapPush(Node, [](const FPathNode& A, const FPathNode& B) { return A.F < B.F; });
	};
	auto HeapPop = [&OpenHeap]() -> FPathNode
	{
		FPathNode Node;
		OpenHeap.HeapPop(Node, [](const FPathNode& A, const FPathNode& B) { return A.F < B.F; });
		return Node;
	};

	BestG.Add(From, 0);
	HeapPush({ From, 0, Heuristic(From) });

	bool bFound = false;

	while (OpenHeap.Num() > 0)
	{
		FPathNode Current = HeapPop();

		if (Current.Pos == To)
		{
			bFound = true;
			break;
		}

		// 이미 더 좋은 경로로 처리된 노드는 건너뜀
		int32* PrevG = BestG.Find(Current.Pos);
		if (PrevG && *PrevG < Current.G) continue;

		for (const FIntPoint& Dir : GSquareDirections)
		{
			FIntPoint Next = Current.Pos + Dir;
			ASquareTile* Tile = GetTile(Next);
			if (Tile == nullptr) continue;
			if (!Tile->IsWalkable()) continue;
			if (!IsEdgePassable(Current.Pos, Next)) continue;

			int32 NewG = Current.G + 1;
			if (NewG > Movement) continue;

			int32* PrevBestG = BestG.Find(Next);
			if (PrevBestG && *PrevBestG <= NewG) continue;

			AGridEntity* Occupant = (Tile->Slot) ? Tile->Slot->SlotEntity : nullptr;
			if (Occupant != nullptr)
			{
				bool bIsFriendly = FriendlyDataClass != nullptr
					&& Occupant->Data != nullptr
					&& Occupant->Data->IsA(FriendlyDataClass);
				if (!bIsFriendly) continue;
			}

			BestG.Add(Next, NewG);
			Predecessors.Add(Next, Current.Pos);
			HeapPush({ Next, NewG, NewG + Heuristic(Next) });
		}
	}

	if (!bFound) return EmptyResult;

	// 경로 역추적
	TArray<FIntPoint> PathPositions;
	FIntPoint CurrentPos = To;
	while (CurrentPos != From)
	{
		PathPositions.Add(CurrentPos);
		FIntPoint* Pred = Predecessors.Find(CurrentPos);
		if (Pred == nullptr) return EmptyResult;
		CurrentPos = *Pred;
	}

	// 역순 → 정순 (from 제외, to 포함)
	TArray<ASquareTile*> Result;
	Result.Reserve(PathPositions.Num());
	for (int32 i = PathPositions.Num() - 1; i >= 0; --i)
	{
		ASquareTile* Tile = GetTile(PathPositions[i]);
		if (Tile) Result.Add(Tile);
	}

	return Result;
}

TArray<ASquareTile*> UGridSubsystem::SmoothPath(const TArray<ASquareTile*>& Path, FIntPoint FromPos, UClass* FriendlyDataClass)
{
	if (Path.Num() <= 1) return Path;

	TArray<ASquareTile*> Result;
	FIntPoint Anchor = FromPos;
	int32 i = 0;

	while (i < Path.Num())
	{
		int32 Furthest = i;
		for (int32 j = Path.Num() - 1; j > i; --j)
		{
			if (HasLineOfSight(Anchor, Path[j]->GridPosition, FriendlyDataClass))
			{
				Furthest = j;
				break;
			}
		}
		Result.Add(Path[Furthest]);
		Anchor = Path[Furthest]->GridPosition;
		i = Furthest + 1;
	}

	return Result;
}

// ════════════════════════════════════════════
// 유틸리티
// ════════════════════════════════════════════

int32 UGridSubsystem::GetDistance(FIntPoint A, FIntPoint B) const
{
	return FMath::Abs(A.X - B.X) + FMath::Abs(A.Y - B.Y);
}

FVector UGridSubsystem::GridToWorld(FIntPoint GridPosition) const
{
	return FVector(
		GridPosition.X * TileSize,
		GridPosition.Y * TileSize,
		0.0f
	);
}

FIntPoint UGridSubsystem::WorldToGrid(FVector WorldPosition) const
{
	return FIntPoint(
		FMath::RoundToInt32(WorldPosition.X / TileSize),
		FMath::RoundToInt32(WorldPosition.Y / TileSize)
	);
}

FVector UGridSubsystem::GetMapCenter() const
{
	if (TilesByPosition.Num() == 0) return FVector::ZeroVector;

	int32 MinX = TNumericLimits<int32>::Max(), MaxX = TNumericLimits<int32>::Min();
	int32 MinY = TNumericLimits<int32>::Max(), MaxY = TNumericLimits<int32>::Min();

	for (const auto& Pair : TilesByPosition)
	{
		FIntPoint Pos = Pair.Key;
		if (Pos.X < MinX) MinX = Pos.X;
		if (Pos.X > MaxX) MaxX = Pos.X;
		if (Pos.Y < MinY) MinY = Pos.Y;
		if (Pos.Y > MaxY) MaxY = Pos.Y;
	}

	return FVector(
		(MinX + MaxX) * 0.5f * TileSize,
		(MinY + MaxY) * 0.5f * TileSize,
		0.0f
	);
}

void UGridSubsystem::GetMapBounds(FVector2D& OutMin, FVector2D& OutMax) const
{
	if (TilesByPosition.Num() == 0)
	{
		OutMin = FVector2D::ZeroVector;
		OutMax = FVector2D::ZeroVector;
		return;
	}

	int32 MinX = TNumericLimits<int32>::Max(), MaxX = TNumericLimits<int32>::Min();
	int32 MinY = TNumericLimits<int32>::Max(), MaxY = TNumericLimits<int32>::Min();

	for (const auto& Pair : TilesByPosition)
	{
		FIntPoint Pos = Pair.Key;
		if (Pos.X < MinX) MinX = Pos.X;
		if (Pos.X > MaxX) MaxX = Pos.X;
		if (Pos.Y < MinY) MinY = Pos.Y;
		if (Pos.Y > MaxY) MaxY = Pos.Y;
	}

	const float HalfTile = TileSize * 0.5f;
	OutMin = FVector2D(MinX * TileSize - HalfTile, MinY * TileSize - HalfTile);
	OutMax = FVector2D(MaxX * TileSize + HalfTile, MaxY * TileSize + HalfTile);
}

void UGridSubsystem::DeselectAllTiles()
{
	for (auto& Pair : TilesByPosition)
	{
		if (Pair.Value)
		{
			Pair.Value->SetState(ETileState::Normal);
		}
	}
}

TArray<FTilePresetData> UGridSubsystem::GetSpawnPoints(UTileMapPreset* Preset, ESpawnPointType Type) const
{
	TArray<FTilePresetData> Result;
	if (Preset == nullptr) return Result;

	for (const FTilePresetData& TileData : Preset->Tiles)
	{
		if (TileData.SpawnPoint == Type)
		{
			Result.Add(TileData);
		}
	}
	return Result;
}

// ════════════════════════════════════════════
// 내부 메서드
// ════════════════════════════════════════════

ASquareTile* UGridSubsystem::CreateTile(FIntPoint GridPosition, ETileType Type)
{
	if (TilesByPosition.Contains(GridPosition)) return nullptr;

	UWorld* World = GetWorld();
	if (World == nullptr) return nullptr;

	FVector WorldPos = GridToWorld(GridPosition);
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ASquareTile* Tile = World->SpawnActor<ASquareTile>(SquareTileClass, WorldPos, FRotator::ZeroRotator, Params);
	if (Tile)
	{
		Tile->Init(NextTileId, GridPosition, Type);
#if WITH_EDITOR
		Tile->SetActorLabel(FString::Printf(TEXT("Tile_%d_%d"), GridPosition.X, GridPosition.Y));
#endif
		TilesByPosition.Add(GridPosition, Tile);
		TilesById.Add(NextTileId, Tile);
		NextTileId++;
	}

	return Tile;
}

void UGridSubsystem::GenerateEdgeTiles()
{
	UWorld* World = GetWorld();
	if (World == nullptr) return;

	for (const auto& Pair : TilesByPosition)
	{
		FIntPoint Pos = Pair.Key;

		for (const FIntPoint& Dir : GSquareDirections)
		{
			FIntPoint NeighborPos = Pos + Dir;
			FEdgeTileKey Key(Pos, NeighborPos);

			if (EdgeTiles.Contains(Key)) continue;
			if (!TilesByPosition.Contains(NeighborPos)) continue;

			CreateEdgeTile(Key, Pos, Dir);
		}
	}
}

AEdgeTile* UGridSubsystem::CreateEdgeTile(const FEdgeTileKey& Key, FIntPoint FromPos, FIntPoint DirOffset)
{
	UWorld* World = GetWorld();
	if (World == nullptr) return nullptr;

	FVector WorldA = GridToWorld(FromPos);
	FVector WorldB = GridToWorld(FromPos + DirOffset);
	FVector EdgeWorldPos = (WorldA + WorldB) * 0.5f;

	ETileDirection Direction = OffsetToDirection(DirOffset);
	FRotator Rotation = FRotator::ZeroRotator;
	if (Direction == ETileDirection::East || Direction == ETileDirection::West)
	{
		Rotation = FRotator(0.0f, 90.0f, 0.0f);
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AEdgeTile* Edge = World->SpawnActor<AEdgeTile>(AEdgeTile::StaticClass(), EdgeWorldPos, Rotation, Params);
	if (Edge)
	{
		Edge->Init(Key, Direction);
#if WITH_EDITOR
		Edge->SetActorLabel(FString::Printf(TEXT("Edge_%s"), *Key.ToString()));
#endif
		EdgeTiles.Add(Key, Edge);
	}

	return Edge;
}

bool UGridSubsystem::IsEdgePassable(FIntPoint A, FIntPoint B) const
{
	FEdgeTileKey Key(A, B);
	const TObjectPtr<AEdgeTile>* Found = EdgeTiles.Find(Key);
	if (Found == nullptr) return true;

	AEdgeTile* Edge = *Found;
	if (Edge->Slot == nullptr) return true;
	if (Edge->Slot->SlotEntity == nullptr) return true;

	ATileWall* Wall = Cast<ATileWall>(Edge->Slot->SlotEntity);
	if (Wall && !Wall->bIsPassable) return false;

	return true;
}

bool UGridSubsystem::HasLineOfSight(FIntPoint From, FIntPoint To, UClass* FriendlyDataClass) const
{
	int32 X = From.X, Y = From.Y;
	int32 DX = FMath::Abs(To.X - From.X);
	int32 DY = FMath::Abs(To.Y - From.Y);
	int32 SX = To.X > From.X ? 1 : -1;
	int32 SY = To.Y > From.Y ? 1 : -1;
	int32 Err = DX - DY;

	while (X != To.X || Y != To.Y)
	{
		int32 PrevX = X, PrevY = Y;
		int32 E2 = 2 * Err;
		bool bMoveX = E2 > -DY;
		bool bMoveY = E2 < DX;

		if (bMoveX) { Err -= DY; X += SX; }
		if (bMoveY) { Err += DX; Y += SY; }

		FIntPoint Pos(X, Y);
		FIntPoint Prev(PrevX, PrevY);

		if (bMoveX && bMoveY)
		{
			// 대각선: 두 코너 경유 경로 중 하나 이상 통과 가능해야 함
			FIntPoint CornerA(X, PrevY);
			FIntPoint CornerB(PrevX, Y);

			ASquareTile* TileA = GetTile(CornerA);
			ASquareTile* TileB = GetTile(CornerB);
			bool bCornerAWalkable = TileA && TileA->IsWalkable();
			bool bCornerBWalkable = TileB && TileB->IsWalkable();

			if (!bCornerAWalkable || !bCornerBWalkable) return false;

			bool bRouteA = IsEdgePassable(Prev, CornerA) && IsEdgePassable(CornerA, Pos);
			bool bRouteB = IsEdgePassable(Prev, CornerB) && IsEdgePassable(CornerB, Pos);

			if (!bRouteA && !bRouteB) return false;
		}
		else
		{
			if (!IsEdgePassable(Prev, Pos)) return false;
		}

		ASquareTile* Tile = GetTile(Pos);
		if (Tile == nullptr) return false;
		if (!Tile->IsWalkable()) return false;

		AGridEntity* Occupant = (Tile->Slot) ? Tile->Slot->SlotEntity : nullptr;
		if (Occupant != nullptr)
		{
			bool bIsFriendly = FriendlyDataClass != nullptr
				&& Occupant->Data != nullptr
				&& Occupant->Data->IsA(FriendlyDataClass);
			if (!bIsFriendly) return false;
		}
	}
	return true;
}

AActor* UGridSubsystem::SpawnEntityFromCatalog(int32 EntityId)
{
	if (EntityId <= 0) return nullptr;
	if (CachedEntityCatalog == nullptr) return nullptr;

	const FEntityCatalogEntry* Entry = CachedEntityCatalog->GetEntry(EntityId);
	if (Entry == nullptr) return nullptr;

	UClass* ActorClass = Entry->ActorClass.LoadSynchronous();
	if (ActorClass == nullptr) return nullptr;

	UWorld* World = GetWorld();
	if (World == nullptr) return nullptr;

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	return World->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);
}
