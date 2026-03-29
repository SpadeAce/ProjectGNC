// Copyright GNC Project. All Rights Reserved.

#include "STileMapEditorWindow.h"
#include "STileMapCanvas.h"
#include "EntityCatalog.h"
#include "Widgets/Input/SNumericEntryBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWidgetSwitcher.h"
#include "Widgets/Layout/SSeparator.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"
#include "Misc/MessageDialog.h"
#include "Dialogs/Dialogs.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"

#define LOCTEXT_NAMESPACE "TileMapEditor"

void STileMapEditorWindow::Construct(const FArguments& InArgs)
{
	RefreshEntityCatalog();

	ChildSlot
	[
		SNew(SVerticalBox)

		// 설정 패널
		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			BuildSettingsPanel()
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
		[
			SNew(SSeparator)
		]

		// 편집 모드 툴바
		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			BuildEditModeToolbar()
		]

		// 도구 패널 (모드별 전환)
		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			BuildToolPanel()
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
		[
			SNew(SSeparator)
		]

		// 그리드 캔버스 (스크롤)
		+ SVerticalBox::Slot().FillHeight(1.0f).Padding(4)
		[
			SNew(SScrollBox)
			.Orientation(Orient_Vertical)
			+ SScrollBox::Slot()
			[
				SNew(SScrollBox)
				.Orientation(Orient_Horizontal)
				+ SScrollBox::Slot()
				[
					SAssignNew(Canvas, STileMapCanvas)
					.OnTileInteraction(this, &STileMapEditorWindow::HandleTileInteraction)
					.OnEdgeInteraction(this, &STileMapEditorWindow::HandleEdgeInteraction)
				]
			]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 2)
		[
			SNew(SSeparator)
		]

		// 저장/불러오기 패널
		+ SVerticalBox::Slot().AutoHeight().Padding(4)
		[
			BuildSaveLoadPanel()
		]
	];
}

// ============================================================
// 레이아웃 빌더
// ============================================================

TSharedRef<SWidget> STileMapEditorWindow::BuildSettingsPanel()
{
	return SNew(SHorizontalBox)

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("폭"))) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SBox).WidthOverride(50)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([this]() { return GridWidth; })
				.OnValueCommitted_Lambda([this](int32 V, ETextCommit::Type) { GridWidth = FMath::Max(1, V); })
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("높이"))) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SBox).WidthOverride(50)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([this]() { return GridHeight; })
				.OnValueCommitted_Lambda([this](int32 V, ETextCommit::Type) { GridHeight = FMath::Max(1, V); })
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("타일크기"))) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SBox).WidthOverride(60)
			[
				SNew(SNumericEntryBox<float>)
				.Value_Lambda([this]() { return TileSize; })
				.OnValueCommitted_Lambda([this](float V, ETextCommit::Type) { TileSize = FMath::Max(0.1f, V); })
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("턴제한"))) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SBox).WidthOverride(50)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([this]() { return TurnLimit; })
				.OnValueCommitted_Lambda([this](int32 V, ETextCommit::Type) { TurnLimit = FMath::Max(0, V); })
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("최대폰수"))) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SBox).WidthOverride(50)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([this]() { return MaxPawnCount; })
				.OnValueCommitted_Lambda([this](int32 V, ETextCommit::Type) { MaxPawnCount = FMath::Max(0, V); })
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("생성")))
			.OnClicked_Lambda([this]() { GenerateGrid(); return FReply::Handled(); })
		];
}

TSharedRef<SWidget> STileMapEditorWindow::BuildEditModeToolbar()
{
	auto MakeModeButton = [this](ETileMapEditMode Mode) -> TSharedRef<SWidget>
	{
		return SNew(SCheckBox)
			.Style(FAppStyle::Get(), "ToggleButtonCheckbox")
			.IsChecked_Lambda([this, Mode]() { return EditMode == Mode ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
			.OnCheckStateChanged_Lambda([this, Mode](ECheckBoxState)
			{
				EditMode = Mode;
				if (ToolSwitcher.IsValid())
				{
					ToolSwitcher->SetActiveWidgetIndex(static_cast<int32>(Mode));
				}
			})
			[
				SNew(STextBlock).Text(GetEditModeLabel(Mode)).Margin(FMargin(8, 2))
			];
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2) [ MakeModeButton(ETileMapEditMode::TileType) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2) [ MakeModeButton(ETileMapEditMode::SpawnPoint) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2) [ MakeModeButton(ETileMapEditMode::Entity) ];
}

TSharedRef<SWidget> STileMapEditorWindow::BuildToolPanel()
{
	return SNew(SVerticalBox)

		+ SVerticalBox::Slot().AutoHeight()
		[
			SAssignNew(ToolSwitcher, SWidgetSwitcher)
			.WidgetIndex(0)

			+ SWidgetSwitcher::Slot() [ BuildTileTypeTools() ]
			+ SWidgetSwitcher::Slot() [ BuildSpawnPointTools() ]
			+ SWidgetSwitcher::Slot() [ BuildEntityTools() ]
		]

		+ SVerticalBox::Slot().AutoHeight().Padding(0, 4)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("전체 채우기")))
				.OnClicked_Lambda([this]() { FillAll(); return FReply::Handled(); })
				.IsEnabled_Lambda([this]() { return bGridInitialized; })
			]
			+ SHorizontalBox::Slot().AutoWidth().Padding(2)
			[
				SNew(SButton)
				.Text(FText::FromString(TEXT("전체 지우기")))
				.OnClicked_Lambda([this]() { ClearAll(); return FReply::Handled(); })
				.IsEnabled_Lambda([this]() { return bGridInitialized; })
			]
		];
}

TSharedRef<SWidget> STileMapEditorWindow::BuildTileTypeTools()
{
	TSharedRef<SHorizontalBox> Box = SNew(SHorizontalBox);

	auto AddTileButton = [&](ETileType Type, const FString& Label)
	{
		Box->AddSlot().AutoWidth().Padding(2)
		[
			SNew(SButton)
			.ButtonColorAndOpacity_Lambda([this, Type]()
			{
				return EditMode == ETileMapEditMode::TileType && SelectedTileType == Type
					? TileMapEditorConstants::GetTileColor(Type)
					: FLinearColor(0.3f, 0.3f, 0.3f);
			})
			.OnClicked_Lambda([this, Type]() { SelectedTileType = Type; return FReply::Handled(); })
			[
				SNew(STextBlock).Text(FText::FromString(Label))
					.ColorAndOpacity_Lambda([this, Type]()
					{
						return SelectedTileType == Type ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor(0.7f, 0.7f, 0.7f));
					})
			]
		];
	};

	AddTileButton(ETileType::Empty, TEXT("Empty"));
	AddTileButton(ETileType::Ground, TEXT("Ground"));
	AddTileButton(ETileType::Dirt, TEXT("Dirt"));
	AddTileButton(ETileType::Water, TEXT("Water"));
	AddTileButton(ETileType::Blocked, TEXT("Blocked"));

	return Box;
}

TSharedRef<SWidget> STileMapEditorWindow::BuildSpawnPointTools()
{
	auto MakeSpawnButton = [this](ESpawnPointType Type, const FString& Label) -> TSharedRef<SWidget>
	{
		return SNew(SButton)
			.ButtonColorAndOpacity_Lambda([this, Type]()
			{
				return SelectedSpawnPoint == Type ? FLinearColor(0.2f, 0.5f, 0.8f) : FLinearColor(0.3f, 0.3f, 0.3f);
			})
			.OnClicked_Lambda([this, Type]() { SelectedSpawnPoint = Type; return FReply::Handled(); })
			[
				SNew(STextBlock).Text(FText::FromString(Label))
			];
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2) [ MakeSpawnButton(ESpawnPointType::Player, TEXT("Player")) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2) [ MakeSpawnButton(ESpawnPointType::Enemy, TEXT("Enemy")) ]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0, 2, 0)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("SpawnId"))) ]
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SBox).WidthOverride(50)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([this]() { return SelectedSpawnId; })
				.OnValueCommitted_Lambda([this](int32 V, ETextCommit::Type) { SelectedSpawnId = V; })
			]
		];
}

TSharedRef<SWidget> STileMapEditorWindow::BuildEntityTools()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(2)
		[ SNew(STextBlock).Text(FText::FromString(TEXT("Entity Id"))) ]

		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SBox).WidthOverride(60)
			[
				SNew(SNumericEntryBox<int32>)
				.Value_Lambda([this]() { return SelectedEntityId; })
				.OnValueCommitted_Lambda([this](int32 V, ETextCommit::Type) { SelectedEntityId = FMath::Max(0, V); })
			]
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("카탈로그에서 선택...")))
			.OnClicked_Lambda([this]()
			{
				ShowEntityPopup([this](int32 Id) { SelectedEntityId = Id; });
				return FReply::Handled();
			})
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(4, 0)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("카탈로그 새로고침")))
			.OnClicked_Lambda([this]() { RefreshEntityCatalog(); return FReply::Handled(); })
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (!CachedCatalog)
					return FText::FromString(TEXT("[카탈로그 없음]"));
				return FText::FromString(FString::Printf(TEXT("[%d개 엔티티]"), CachedCatalog->Entries.Num()));
			})
			.ColorAndOpacity_Lambda([this]()
			{
				return CachedCatalog ? FSlateColor(FLinearColor(0.5f, 0.8f, 0.5f)) : FSlateColor(FLinearColor(0.8f, 0.4f, 0.4f));
			})
		];
}

TSharedRef<SWidget> STileMapEditorWindow::BuildSaveLoadPanel()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("내보내기 (DataAsset)")))
			.OnClicked_Lambda([this]() { ExportAsDataAsset(); return FReply::Handled(); })
			.IsEnabled_Lambda([this]() { return bGridInitialized; })
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("불러오기")))
			.OnClicked_Lambda([this]() { LoadPreset(); return FReply::Handled(); })
		]

		+ SHorizontalBox::Slot().AutoWidth().Padding(2)
		[
			SNew(SButton)
			.Text(FText::FromString(TEXT("저장 (덮어쓰기)")))
			.OnClicked_Lambda([this]() { SaveOverwrite(); return FReply::Handled(); })
			.IsEnabled_Lambda([this]() { return bGridInitialized && LoadedPreset.IsValid(); })
		]

		+ SHorizontalBox::Slot().AutoWidth().VAlign(VAlign_Center).Padding(8, 0)
		[
			SNew(STextBlock)
			.Text_Lambda([this]()
			{
				if (LoadedPreset.IsValid())
					return FText::FromString(LoadedPreset->GetName());
				return FText::FromString(TEXT("[프리셋 없음]"));
			})
		];
}

// ============================================================
// 그리드 조작
// ============================================================

void STileMapEditorWindow::GenerateGrid()
{
	if (GridWidth <= 0 || GridHeight <= 0) return;

	GridData.Empty();
	GridData.SetNum(GridWidth * GridHeight);
	EdgeEntityData.Empty();

	for (int32 X = 0; X < GridWidth; ++X)
	{
		for (int32 Y = 0; Y < GridHeight; ++Y)
		{
			FTilePresetData& Tile = GetTileDataRef(X, Y);
			Tile.Position = FIntPoint(X, Y);
			Tile.TileType = ETileType::Ground;
			Tile.SpawnPoint = ESpawnPointType::None;
			Tile.SpawnId = 0;
			Tile.EntityId = 0;
		}
	}

	bGridInitialized = true;
	LoadedPreset = nullptr;
	RefreshCanvas();
}

void STileMapEditorWindow::FillAll()
{
	if (!bGridInitialized) return;

	switch (EditMode)
	{
	case ETileMapEditMode::TileType:
		for (FTilePresetData& Tile : GridData)
			Tile.TileType = SelectedTileType;
		break;

	case ETileMapEditMode::SpawnPoint:
		for (FTilePresetData& Tile : GridData)
		{
			Tile.SpawnPoint = SelectedSpawnPoint;
			Tile.SpawnId = SelectedSpawnId;
		}
		break;

	case ETileMapEditMode::Entity:
		for (FTilePresetData& Tile : GridData)
			Tile.EntityId = SelectedEntityId;
		break;
	}

	RefreshCanvas();
}

void STileMapEditorWindow::ClearAll()
{
	if (!bGridInitialized) return;

	for (FTilePresetData& Tile : GridData)
	{
		Tile.TileType = ETileType::Ground;
		Tile.SpawnPoint = ESpawnPointType::None;
		Tile.SpawnId = 0;
		Tile.EntityId = 0;
	}
	EdgeEntityData.Empty();
	RefreshCanvas();
}

// ============================================================
// 캔버스 콜백
// ============================================================

void STileMapEditorWindow::HandleTileInteraction(int32 X, int32 Y, bool bIsDrag)
{
	if (!bGridInitialized) return;
	if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight) return;

	switch (EditMode)
	{
	case ETileMapEditMode::TileType:
		ApplyTileTypePaint(X, Y);
		break;
	case ETileMapEditMode::SpawnPoint:
		ApplySpawnPoint(X, Y, bIsDrag);
		break;
	case ETileMapEditMode::Entity:
		if (!bIsDrag) ApplyEntityToTile(X, Y);
		break;
	}

	RefreshCanvas();
}

void STileMapEditorWindow::HandleEdgeInteraction(FEdgeTileKey Key, bool bIsDrag)
{
	if (!bGridInitialized) return;

	if (EditMode == ETileMapEditMode::Entity && !bIsDrag)
	{
		ApplyEntityToEdge(Key);
		RefreshCanvas();
	}
}

// ============================================================
// 편집 로직
// ============================================================

void STileMapEditorWindow::ApplyTileTypePaint(int32 X, int32 Y)
{
	GetTileDataRef(X, Y).TileType = SelectedTileType;
}

void STileMapEditorWindow::ApplySpawnPoint(int32 X, int32 Y, bool bIsDrag)
{
	FTilePresetData& Tile = GetTileDataRef(X, Y);

	if (bIsDrag)
	{
		// 드래그: 항상 선택된 타입/ID로 페인트
		Tile.SpawnPoint = SelectedSpawnPoint;
		Tile.SpawnId = SelectedSpawnId;
	}
	else
	{
		// 클릭: 토글
		if (Tile.SpawnPoint == ESpawnPointType::None)
		{
			Tile.SpawnPoint = SelectedSpawnPoint;
			Tile.SpawnId = SelectedSpawnId;
		}
		else
		{
			Tile.SpawnPoint = ESpawnPointType::None;
			Tile.SpawnId = 0;
		}
	}
}

void STileMapEditorWindow::ApplyEntityToTile(int32 X, int32 Y)
{
	ShowEntityPopup([this, X, Y](int32 Id)
	{
		GetTileDataRef(X, Y).EntityId = Id;
		RefreshCanvas();
	});
}

void STileMapEditorWindow::ApplyEntityToEdge(FEdgeTileKey Key)
{
	ShowEntityPopup([this, Key](int32 Id)
	{
		if (Id > 0)
			EdgeEntityData.Add(Key, Id);
		else
			EdgeEntityData.Remove(Key);
		RefreshCanvas();
	});
}

void STileMapEditorWindow::ShowEntityPopup(TFunction<void(int32)> OnSelected)
{
	FMenuBuilder MenuBuilder(true, nullptr);

	// "None" 옵션
	MenuBuilder.AddMenuEntry(
		FText::FromString(TEXT("None")),
		FText::GetEmpty(),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([OnSelected]() { OnSelected(0); }))
	);

	MenuBuilder.AddSeparator();

	if (CachedCatalog)
	{
		for (const FEntityCatalogEntry& Entry : CachedCatalog->Entries)
		{
			const int32 EntryId = Entry.Id;
			const FString Label = FString::Printf(TEXT("[%s] %s (ID:%d)"),
				*UEnum::GetValueAsString(Entry.Category).RightChop(18), // "EEntityCategory::" 제거
				*Entry.DisplayName,
				Entry.Id);

			MenuBuilder.AddMenuEntry(
				FText::FromString(Label),
				FText::GetEmpty(),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateLambda([OnSelected, EntryId]() { OnSelected(EntryId); }))
			);
		}
	}

	const FVector2D CursorPos = FSlateApplication::Get().GetCursorPos();
	FSlateApplication::Get().PushMenu(
		SharedThis(this),
		FWidgetPath(),
		MenuBuilder.MakeWidget(),
		CursorPos,
		FPopupTransitionEffect::ContextMenu);
}

// ============================================================
// 저장/불러오기
// ============================================================

void STileMapEditorWindow::ExportAsDataAsset()
{
	if (!bGridInitialized) return;

	// 콘텐츠 브라우저 저장 다이얼로그
	FSaveAssetDialogConfig Config;
	Config.DefaultPath = TEXT("/Game/CardGame/DataAssets");
	Config.DefaultAssetName = TEXT("TMP_NewPreset");
	Config.AssetClassNames.Add(UTileMapPreset::StaticClass()->GetClassPathName());
	Config.ExistingAssetPolicy = ESaveAssetDialogExistingAssetPolicy::AllowButWarn;
	Config.DialogTitleOverride = FText::FromString(TEXT("TileMap 프리셋 저장"));

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	const FString SaveObjectPath = ContentBrowserModule.Get().CreateModalSaveAssetDialog(Config);

	if (SaveObjectPath.IsEmpty()) return;

	// 다이얼로그 반환값에서 패키지 경로와 에셋 이름을 안전하게 분리
	const FString PackageName = FPackageName::ObjectPathToPackageName(SaveObjectPath);
	const FString AssetName = FPaths::GetBaseFilename(PackageName);

	UE_LOG(LogTemp, Log, TEXT("[TileMapEditor] SaveObjectPath=%s, PackageName=%s, AssetName=%s"),
		*SaveObjectPath, *PackageName, *AssetName);

	if (AssetName.IsEmpty())
	{
		FMessageDialog::Open(EAppMsgType::Ok,
			FText::FromString(TEXT("에셋 이름이 비어있습니다.")));
		return;
	}

	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	// 같은 이름의 기존 오브젝트가 있으면 제거
	UTileMapPreset* ExistingObject = FindObject<UTileMapPreset>(Package, *AssetName);
	if (ExistingObject)
	{
		ExistingObject->Rename(nullptr, GetTransientPackage(), REN_DontCreateRedirectors);
	}

	UTileMapPreset* NewPreset = NewObject<UTileMapPreset>(
		Package, FName(*AssetName), RF_Public | RF_Standalone);
	FillPresetFromGrid(NewPreset);

	FAssetRegistryModule::AssetCreated(NewPreset);
	NewPreset->MarkPackageDirty();

	const FString PackageFilename = FPackageName::LongPackageNameToFilename(
		PackageName, FPackageName::GetAssetPackageExtension());

	UE_LOG(LogTemp, Log, TEXT("[TileMapEditor] Saving to: %s"), *PackageFilename);

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	const bool bSaved = UPackage::SavePackage(
		Package, NewPreset, *PackageFilename, SaveArgs);

	if (bSaved)
	{
		LoadedPreset = NewPreset;
		UE_LOG(LogTemp, Log, TEXT("[TileMapEditor] 저장 성공: %s"), *PackageName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[TileMapEditor] 저장 실패: %s"), *PackageName);
	}
}

void STileMapEditorWindow::LoadPreset()
{
	FOpenAssetDialogConfig Config;
	Config.DefaultPath = TEXT("/Game/CardGame/DataAssets");
	Config.AssetClassNames.Add(UTileMapPreset::StaticClass()->GetClassPathName());
	Config.DialogTitleOverride = FText::FromString(TEXT("TileMap 프리셋 불러오기"));
	Config.bAllowMultipleSelection = false;

	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	const TArray<FAssetData> Assets = ContentBrowserModule.Get().CreateModalOpenAssetDialog(Config);

	if (Assets.Num() == 0) return;

	UTileMapPreset* Preset = Cast<UTileMapPreset>(Assets[0].GetAsset());
	if (!Preset) return;

	ApplyPresetToGrid(Preset);
	LoadedPreset = Preset;
}

void STileMapEditorWindow::SaveOverwrite()
{
	if (!bGridInitialized || !LoadedPreset.IsValid()) return;

	FillPresetFromGrid(LoadedPreset.Get());
	LoadedPreset->MarkPackageDirty();

	UPackage* Package = LoadedPreset->GetOutermost();
	const FString PackageFilename = FPackageName::LongPackageNameToFilename(
		Package->GetName(), FPackageName::GetAssetPackageExtension());
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;
	UPackage::SavePackage(Package, LoadedPreset.Get(), *PackageFilename, SaveArgs);
}

void STileMapEditorWindow::ApplyPresetToGrid(const UTileMapPreset* Preset)
{
	GridWidth = Preset->Width;
	GridHeight = Preset->Height;
	TileSize = Preset->TileSize;
	TurnLimit = Preset->TurnLimit;
	MaxPawnCount = Preset->MaxPawnCount;

	// 그리드 초기화
	GridData.Empty();
	GridData.SetNum(GridWidth * GridHeight);
	EdgeEntityData.Empty();

	for (int32 X = 0; X < GridWidth; ++X)
	{
		for (int32 Y = 0; Y < GridHeight; ++Y)
		{
			FTilePresetData& Tile = GetTileDataRef(X, Y);
			Tile.Position = FIntPoint(X, Y);
			Tile.TileType = ETileType::Ground;
			Tile.SpawnPoint = ESpawnPointType::None;
			Tile.SpawnId = 0;
			Tile.EntityId = 0;
		}
	}

	// 프리셋 타일 데이터 적용
	for (const FTilePresetData& PresetTile : Preset->Tiles)
	{
		const int32 X = PresetTile.Position.X;
		const int32 Y = PresetTile.Position.Y;
		if (X >= 0 && X < GridWidth && Y >= 0 && Y < GridHeight)
		{
			GetTileDataRef(X, Y) = PresetTile;
		}
	}

	// 엣지 데이터 적용
	for (const FEdgePresetData& Edge : Preset->Edges)
	{
		if (Edge.EntityId > 0)
		{
			FEdgeTileKey Key(Edge.PosA, Edge.PosB);
			EdgeEntityData.Add(Key, Edge.EntityId);
		}
	}

	bGridInitialized = true;
	RefreshCanvas();
}

void STileMapEditorWindow::FillPresetFromGrid(UTileMapPreset* Preset) const
{
	Preset->Width = GridWidth;
	Preset->Height = GridHeight;
	Preset->TileSize = TileSize;
	Preset->TurnLimit = TurnLimit;
	Preset->MaxPawnCount = MaxPawnCount;

	Preset->Tiles.Empty();
	Preset->Tiles.Reserve(GridData.Num());
	for (const FTilePresetData& Tile : GridData)
	{
		Preset->Tiles.Add(Tile);
	}

	Preset->Edges.Empty();
	for (const auto& Pair : EdgeEntityData)
	{
		if (Pair.Value > 0)
		{
			FEdgePresetData Edge;
			Edge.PosA = Pair.Key.PosA;
			Edge.PosB = Pair.Key.PosB;
			Edge.EntityId = Pair.Value;
			Preset->Edges.Add(Edge);
		}
	}
}

// ============================================================
// 유틸리티
// ============================================================

void STileMapEditorWindow::RefreshEntityCatalog()
{
	CachedCatalog = nullptr;

	FAssetRegistryModule& RegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
	IAssetRegistry& Registry = RegistryModule.Get();

	TArray<FAssetData> AssetList;
	Registry.GetAssetsByClass(UEntityCatalog::StaticClass()->GetClassPathName(), AssetList);

	if (AssetList.Num() > 0)
	{
		CachedCatalog = Cast<UEntityCatalog>(AssetList[0].GetAsset());
	}
}

void STileMapEditorWindow::RefreshCanvas()
{
	if (Canvas.IsValid())
	{
		Canvas->SetGridData(GridWidth, GridHeight, &GridData, &EdgeEntityData, CachedCatalog.Get());
	}
}

FTilePresetData& STileMapEditorWindow::GetTileDataRef(int32 X, int32 Y)
{
	return GridData[GetTileIndex(X, Y)];
}

FText STileMapEditorWindow::GetEditModeLabel(ETileMapEditMode Mode) const
{
	switch (Mode)
	{
	case ETileMapEditMode::TileType:   return FText::FromString(TEXT("타일 타입"));
	case ETileMapEditMode::SpawnPoint: return FText::FromString(TEXT("스폰 포인트"));
	case ETileMapEditMode::Entity:     return FText::FromString(TEXT("엔티티 배치"));
	default: return FText::GetEmpty();
	}
}

#undef LOCTEXT_NAMESPACE
