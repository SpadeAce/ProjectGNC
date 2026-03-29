// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "TileMapEditorTypes.h"
#include "TileMapPreset.h"

class UEntityCatalog;

DECLARE_DELEGATE_ThreeParams(FOnTileInteraction, int32 /*X*/, int32 /*Y*/, bool /*bIsDrag*/);
DECLARE_DELEGATE_TwoParams(FOnEdgeInteraction, FEdgeTileKey /*Key*/, bool /*bIsDrag*/);

/**
 * STileMapCanvas
 * 타일맵 그리드를 커스텀 렌더링하고 마우스 입력을 처리하는 캔버스 위젯.
 */
class STileMapCanvas : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STileMapCanvas) {}
	SLATE_EVENT(FOnTileInteraction, OnTileInteraction)
	SLATE_EVENT(FOnEdgeInteraction, OnEdgeInteraction)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	// 외부에서 그리드 데이터 참조를 설정
	void SetGridData(
		int32 InWidth, int32 InHeight,
		const TArray<FTilePresetData>* InGridData,
		const TMap<FEdgeTileKey, int32>* InEdgeData,
		const UEntityCatalog* InCatalog);

	void RequestRedraw();

protected:
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

private:
	FTileMapHitResult HitTest(FVector2D LocalPos) const;
	FString GetEntityShortName(int32 EntityId) const;

	// 렌더링 서브루틴
	void PaintTileCells(const FGeometry& Geom, FSlateWindowElementList& OutDrawElements,
		int32 LayerId) const;
	void PaintEdgeSlots(const FGeometry& Geom, FSlateWindowElementList& OutDrawElements,
		int32 LayerId) const;

	// 델리게이트
	FOnTileInteraction OnTileInteraction;
	FOnEdgeInteraction OnEdgeInteraction;

	// 데이터 참조 (소유하지 않음)
	int32 GridWidth = 0;
	int32 GridHeight = 0;
	const TArray<FTilePresetData>* GridData = nullptr;
	const TMap<FEdgeTileKey, int32>* EdgeData = nullptr;
	const UEntityCatalog* Catalog = nullptr;

	// 페인팅 상태
	bool bIsPainting = false;
};
