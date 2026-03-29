// Copyright GNC Project. All Rights Reserved.

#include "STileMapCanvas.h"
#include "EntityCatalog.h"
#include "Rendering/DrawElements.h"
#include "Fonts/FontMeasure.h"
#include "Framework/Application/SlateApplication.h"

void STileMapCanvas::Construct(const FArguments& InArgs)
{
	OnTileInteraction = InArgs._OnTileInteraction;
	OnEdgeInteraction = InArgs._OnEdgeInteraction;
}

void STileMapCanvas::SetGridData(
	int32 InWidth, int32 InHeight,
	const TArray<FTilePresetData>* InGridData,
	const TMap<FEdgeTileKey, int32>* InEdgeData,
	const UEntityCatalog* InCatalog)
{
	GridWidth = InWidth;
	GridHeight = InHeight;
	GridData = InGridData;
	EdgeData = InEdgeData;
	Catalog = InCatalog;
	Invalidate(EInvalidateWidgetReason::Paint);
}

void STileMapCanvas::RequestRedraw()
{
	Invalidate(EInvalidateWidgetReason::Paint);
}

FVector2D STileMapCanvas::ComputeDesiredSize(float) const
{
	if (GridWidth <= 0 || GridHeight <= 0)
	{
		return FVector2D(100.0f, 100.0f);
	}

	using namespace TileMapEditorConstants;
	const float W = GridWidth * CellSize + FMath::Max(0, GridWidth - 1) * EdgeSize;
	const float H = GridHeight * CellSize + FMath::Max(0, GridHeight - 1) * EdgeSize;
	return FVector2D(W, H);
}

int32 STileMapCanvas::OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
	const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
	int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const
{
	if (!GridData || GridWidth <= 0 || GridHeight <= 0)
	{
		return LayerId;
	}

	PaintTileCells(AllottedGeometry, OutDrawElements, LayerId);
	PaintEdgeSlots(AllottedGeometry, OutDrawElements, LayerId);

	return LayerId;
}

void STileMapCanvas::PaintTileCells(const FGeometry& Geom, FSlateWindowElementList& OutDrawElements,
	int32 LayerId) const
{
	using namespace TileMapEditorConstants;

	const FSlateBrush* WhiteBrush = FAppStyle::Get().GetBrush(TEXT("WhiteBrush"));
	const FSlateFontInfo SpawnFont = FCoreStyle::GetDefaultFontStyle("Bold", 9);
	const FSlateFontInfo EntityFont = FCoreStyle::GetDefaultFontStyle("Regular", 7);
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	for (int32 DisplayRow = 0; DisplayRow < GridHeight; ++DisplayRow)
	{
		const int32 DataY = GridHeight - 1 - DisplayRow;

		for (int32 Col = 0; Col < GridWidth; ++Col)
		{
			const int32 Index = Col * GridHeight + DataY;
			if (!GridData->IsValidIndex(Index)) continue;

			const FTilePresetData& Tile = (*GridData)[Index];
			const float PosX = Col * CellStride;
			const float PosY = DisplayRow * CellStride;
			const FVector2D CellPos(PosX, PosY);
			const FVector2D CellSizeVec(CellSize, CellSize);

			// 타일 배경
			const FLinearColor TileColor = GetTileColor(Tile.TileType);
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
				Geom.ToPaintGeometry(CellSizeVec, FSlateLayoutTransform(CellPos)),
				WhiteBrush, ESlateDrawEffect::None, TileColor);

			// 테두리 (4변)
			const float Border = 1.0f;
			// Top
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
				Geom.ToPaintGeometry(FVector2D(CellSize, Border), FSlateLayoutTransform(CellPos)),
				WhiteBrush, ESlateDrawEffect::None, BorderColor);
			// Bottom
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
				Geom.ToPaintGeometry(FVector2D(CellSize, Border), FSlateLayoutTransform(FVector2D(PosX, PosY + CellSize - Border))),
				WhiteBrush, ESlateDrawEffect::None, BorderColor);
			// Left
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
				Geom.ToPaintGeometry(FVector2D(Border, CellSize), FSlateLayoutTransform(CellPos)),
				WhiteBrush, ESlateDrawEffect::None, BorderColor);
			// Right
			FSlateDrawElement::MakeBox(OutDrawElements, LayerId + 1,
				Geom.ToPaintGeometry(FVector2D(Border, CellSize), FSlateLayoutTransform(FVector2D(PosX + CellSize - Border, PosY))),
				WhiteBrush, ESlateDrawEffect::None, BorderColor);

			// 스폰 포인트 라벨 (우상단)
			if (Tile.SpawnPoint != ESpawnPointType::None)
			{
				const FString Label = (Tile.SpawnPoint == ESpawnPointType::Player)
					? FString::Printf(TEXT("P:%d"), Tile.SpawnId)
					: FString::Printf(TEXT("E:%d"), Tile.SpawnId);
				const FLinearColor& LabelColor = (Tile.SpawnPoint == ESpawnPointType::Player)
					? SpawnPlayerColor : SpawnEnemyColor;

				const FVector2D TextSize = FontMeasure->Measure(Label, SpawnFont);
				const FVector2D TextPos(PosX + CellSize - TextSize.X - 1.0f, PosY + 1.0f);

				FSlateDrawElement::MakeText(OutDrawElements, LayerId + 2,
					Geom.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
					Label, SpawnFont, ESlateDrawEffect::None, LabelColor);
			}

			// 엔티티 라벨 (하단 중앙)
			if (Tile.EntityId > 0)
			{
				const FString ShortName = GetEntityShortName(Tile.EntityId);
				const FVector2D TextSize = FontMeasure->Measure(ShortName, EntityFont);
				const FVector2D TextPos(
					PosX + (CellSize - TextSize.X) * 0.5f,
					PosY + CellSize - TextSize.Y - 1.0f);

				FSlateDrawElement::MakeText(OutDrawElements, LayerId + 2,
					Geom.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
					ShortName, EntityFont, ESlateDrawEffect::None, FLinearColor::White);
			}
		}
	}
}

void STileMapCanvas::PaintEdgeSlots(const FGeometry& Geom, FSlateWindowElementList& OutDrawElements,
	int32 LayerId) const
{
	using namespace TileMapEditorConstants;

	const FSlateBrush* WhiteBrush = FAppStyle::Get().GetBrush(TEXT("WhiteBrush"));
	const FSlateFontInfo EdgeFont = FCoreStyle::GetDefaultFontStyle("Regular", 7);
	const TSharedRef<FSlateFontMeasure> FontMeasure = FSlateApplication::Get().GetRenderer()->GetFontMeasureService();

	// East 엣지 (세로 스트립: 열 사이)
	for (int32 Col = 0; Col < GridWidth - 1; ++Col)
	{
		for (int32 DisplayRow = 0; DisplayRow < GridHeight; ++DisplayRow)
		{
			const int32 DataY = GridHeight - 1 - DisplayRow;
			const FIntPoint PosA(Col, DataY);
			const FIntPoint PosB(Col + 1, DataY);
			const FEdgeTileKey Key(PosA, PosB);

			const float PosX = Col * CellStride + CellSize;
			const float PosY = DisplayRow * CellStride;

			const int32* EntityIdPtr = EdgeData ? EdgeData->Find(Key) : nullptr;
			const bool bOccupied = EntityIdPtr && *EntityIdPtr > 0;

			FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
				Geom.ToPaintGeometry(FVector2D(EdgeSize, CellSize), FSlateLayoutTransform(FVector2D(PosX, PosY))),
				WhiteBrush, ESlateDrawEffect::None,
				bOccupied ? EdgeOccupiedColor : EdgeEmptyColor);

			if (bOccupied)
			{
				const FString ShortName = GetEntityShortName(*EntityIdPtr);
				const FVector2D TextSize = FontMeasure->Measure(ShortName, EdgeFont);
				const FVector2D TextPos(
					PosX + (EdgeSize - TextSize.X) * 0.5f,
					PosY + (CellSize - TextSize.Y) * 0.5f);

				FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
					Geom.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
					ShortName, EdgeFont, ESlateDrawEffect::None, FLinearColor::Black);
			}
		}
	}

	// North 엣지 (가로 스트립: 행 사이)
	for (int32 DisplayRow = 0; DisplayRow < GridHeight - 1; ++DisplayRow)
	{
		const int32 DataYLower = GridHeight - 2 - DisplayRow;

		for (int32 Col = 0; Col < GridWidth; ++Col)
		{
			const FIntPoint PosA(Col, DataYLower);
			const FIntPoint PosB(Col, DataYLower + 1);
			const FEdgeTileKey Key(PosA, PosB);

			const float PosX = Col * CellStride;
			const float PosY = DisplayRow * CellStride + CellSize;

			const int32* EntityIdPtr = EdgeData ? EdgeData->Find(Key) : nullptr;
			const bool bOccupied = EntityIdPtr && *EntityIdPtr > 0;

			FSlateDrawElement::MakeBox(OutDrawElements, LayerId,
				Geom.ToPaintGeometry(FVector2D(CellSize, EdgeSize), FSlateLayoutTransform(FVector2D(PosX, PosY))),
				WhiteBrush, ESlateDrawEffect::None,
				bOccupied ? EdgeOccupiedColor : EdgeEmptyColor);

			if (bOccupied)
			{
				const FString ShortName = GetEntityShortName(*EntityIdPtr);
				const FVector2D TextSize = FontMeasure->Measure(ShortName, EdgeFont);
				const FVector2D TextPos(
					PosX + (CellSize - TextSize.X) * 0.5f,
					PosY + (EdgeSize - TextSize.Y) * 0.5f);

				FSlateDrawElement::MakeText(OutDrawElements, LayerId + 1,
					Geom.ToPaintGeometry(TextSize, FSlateLayoutTransform(TextPos)),
					ShortName, EdgeFont, ESlateDrawEffect::None, FLinearColor::Black);
			}
		}
	}
}

// ── 마우스 입력 ──

FReply STileMapCanvas::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && GridData)
	{
		bIsPainting = true;
		const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		const FTileMapHitResult Hit = HitTest(LocalPos);

		if (Hit.HitType == ETileMapHitType::Tile)
		{
			OnTileInteraction.ExecuteIfBound(Hit.TileCoord.X, Hit.TileCoord.Y, false);
		}
		else if (Hit.HitType == ETileMapHitType::EastEdge || Hit.HitType == ETileMapHitType::NorthEdge)
		{
			OnEdgeInteraction.ExecuteIfBound(Hit.EdgeKey, false);
		}

		return FReply::Handled().CaptureMouse(SharedThis(this));
	}
	return FReply::Unhandled();
}

FReply STileMapCanvas::OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bIsPainting && GridData)
	{
		const FVector2D LocalPos = MyGeometry.AbsoluteToLocal(MouseEvent.GetScreenSpacePosition());
		const FTileMapHitResult Hit = HitTest(LocalPos);

		if (Hit.HitType == ETileMapHitType::Tile)
		{
			OnTileInteraction.ExecuteIfBound(Hit.TileCoord.X, Hit.TileCoord.Y, true);
		}
		else if (Hit.HitType == ETileMapHitType::EastEdge || Hit.HitType == ETileMapHitType::NorthEdge)
		{
			OnEdgeInteraction.ExecuteIfBound(Hit.EdgeKey, true);
		}

		return FReply::Handled();
	}
	return FReply::Unhandled();
}

FReply STileMapCanvas::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPainting = false;
		return FReply::Handled().ReleaseMouseCapture();
	}
	return FReply::Unhandled();
}

FTileMapHitResult STileMapCanvas::HitTest(FVector2D LocalPos) const
{
	using namespace TileMapEditorConstants;

	FTileMapHitResult Result;

	if (LocalPos.X < 0 || LocalPos.Y < 0) return Result;

	const int32 Col = FMath::FloorToInt32(LocalPos.X / CellStride);
	const int32 Row = FMath::FloorToInt32(LocalPos.Y / CellStride);
	const float ColPart = FMath::Fmod(LocalPos.X, CellStride);
	const float RowPart = FMath::Fmod(LocalPos.Y, CellStride);

	const bool bInTileCol = ColPart < CellSize;
	const bool bInTileRow = RowPart < CellSize;

	if (bInTileCol && bInTileRow)
	{
		// 타일 셀 히트
		if (Col >= 0 && Col < GridWidth && Row >= 0 && Row < GridHeight)
		{
			Result.HitType = ETileMapHitType::Tile;
			Result.TileCoord = FIntPoint(Col, GridHeight - 1 - Row);
		}
	}
	else if (!bInTileCol && bInTileRow)
	{
		// East 엣지 히트 (세로 스트립)
		if (Col >= 0 && Col < GridWidth - 1 && Row >= 0 && Row < GridHeight)
		{
			const int32 DataY = GridHeight - 1 - Row;
			Result.HitType = ETileMapHitType::EastEdge;
			Result.EdgeKey = FEdgeTileKey(FIntPoint(Col, DataY), FIntPoint(Col + 1, DataY));
		}
	}
	else if (bInTileCol && !bInTileRow)
	{
		// North 엣지 히트 (가로 스트립)
		if (Col >= 0 && Col < GridWidth && Row >= 0 && Row < GridHeight - 1)
		{
			const int32 DataYLower = GridHeight - 2 - Row;
			Result.HitType = ETileMapHitType::NorthEdge;
			Result.EdgeKey = FEdgeTileKey(FIntPoint(Col, DataYLower), FIntPoint(Col, DataYLower + 1));
		}
	}

	return Result;
}

FString STileMapCanvas::GetEntityShortName(int32 EntityId) const
{
	if (Catalog)
	{
		const FEntityCatalogEntry* Entry = Catalog->GetEntry(EntityId);
		if (Entry && !Entry->DisplayName.IsEmpty())
		{
			return Entry->DisplayName.Left(6);
		}
	}
	return FString::Printf(TEXT("#%d"), EntityId);
}
