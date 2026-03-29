// Copyright GNC Project. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "TileMapEditorTypes.h"
#include "TileMapPreset.h"

class STileMapCanvas;
class UEntityCatalog;
struct FEntityCatalogEntry;

/**
 * STileMapEditorWindow
 * 타일맵 프리셋 에디터의 메인 위젯.
 * 설정 패널, 편집 모드 전환, 도구 패널, 그리드 캔버스, 저장/불러오기를 통합 관리.
 */
class STileMapEditorWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(STileMapEditorWindow) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

private:
	// ── 레이아웃 빌더 ──
	TSharedRef<SWidget> BuildSettingsPanel();
	TSharedRef<SWidget> BuildEditModeToolbar();
	TSharedRef<SWidget> BuildToolPanel();
	TSharedRef<SWidget> BuildSaveLoadPanel();

	TSharedRef<SWidget> BuildTileTypeTools();
	TSharedRef<SWidget> BuildSpawnPointTools();
	TSharedRef<SWidget> BuildEntityTools();

	// ── 그리드 조작 ──
	void GenerateGrid();
	void FillAll();
	void ClearAll();

	// ── 캔버스 콜백 ──
	void HandleTileInteraction(int32 X, int32 Y, bool bIsDrag);
	void HandleEdgeInteraction(FEdgeTileKey Key, bool bIsDrag);

	// ── 편집 로직 ──
	void ApplyTileTypePaint(int32 X, int32 Y);
	void ApplySpawnPoint(int32 X, int32 Y, bool bIsDrag);
	void ApplyEntityToTile(int32 X, int32 Y);
	void ApplyEntityToEdge(FEdgeTileKey Key);
	void ShowEntityPopup(TFunction<void(int32)> OnSelected);

	// ── 저장/불러오기 ──
	void ExportAsDataAsset();
	void LoadPreset();
	void SaveOverwrite();
	void ApplyPresetToGrid(const UTileMapPreset* Preset);
	void FillPresetFromGrid(UTileMapPreset* Preset) const;

	// ── 유틸리티 ──
	void RefreshEntityCatalog();
	void RefreshCanvas();
	FTilePresetData& GetTileDataRef(int32 X, int32 Y);
	int32 GetTileIndex(int32 X, int32 Y) const { return X * GridHeight + Y; }
	FText GetEditModeLabel(ETileMapEditMode Mode) const;

	// ── 상태 ──
	int32 GridWidth = 5;
	int32 GridHeight = 5;
	float TileSize = 1.05f;
	int32 TurnLimit = 20;
	int32 MaxPawnCount = 4;
	bool bGridInitialized = false;

	ETileMapEditMode EditMode = ETileMapEditMode::TileType;
	ETileType SelectedTileType = ETileType::Ground;
	ESpawnPointType SelectedSpawnPoint = ESpawnPointType::Player;
	int32 SelectedSpawnId = 1;
	int32 SelectedEntityId = 0;

	TArray<FTilePresetData> GridData;
	TMap<FEdgeTileKey, int32> EdgeEntityData;

	TWeakObjectPtr<UTileMapPreset> LoadedPreset;
	TObjectPtr<UEntityCatalog> CachedCatalog;

	// ── 위젯 참조 ──
	TSharedPtr<STileMapCanvas> Canvas;
	TSharedPtr<SWidgetSwitcher> ToolSwitcher;
};
