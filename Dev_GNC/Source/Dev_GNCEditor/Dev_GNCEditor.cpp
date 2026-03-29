// Copyright GNC Project. All Rights Reserved.

#include "Dev_GNCEditor.h"
#include "STileMapEditorWindow.h"
#include "ToolMenus.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Docking/TabManager.h"

IMPLEMENT_MODULE(FDev_GNCEditorModule, Dev_GNCEditor)

const FName FDev_GNCEditorModule::TileMapEditorTabName(TEXT("TileMapEditorTab"));

void FDev_GNCEditorModule::StartupModule()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		TileMapEditorTabName,
		FOnSpawnTab::CreateRaw(this, &FDev_GNCEditorModule::SpawnTileMapEditorTab))
		.SetDisplayName(FText::FromString(TEXT("TileMap Editor")))
		.SetMenuType(ETabSpawnerMenuType::Enabled);

	UToolMenus::RegisterStartupCallback(
		FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FDev_GNCEditorModule::RegisterMenus));
}

void FDev_GNCEditorModule::ShutdownModule()
{
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(TileMapEditorTabName);

	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
}

void FDev_GNCEditorModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);

	UToolMenu* Menu = UToolMenus::Get()->ExtendMenu(TEXT("LevelEditor.MainMenu.Tools"));
	FToolMenuSection& Section = Menu->FindOrAddSection(TEXT("CardGameTools"));
	Section.Label = FText::FromString(TEXT("CardGame"));

	Section.AddMenuEntry(
		TEXT("OpenTileMapEditor"),
		FText::FromString(TEXT("TileMap Editor")),
		FText::FromString(TEXT("타일맵 프리셋 에디터를 엽니다")),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateLambda([]()
		{
			FGlobalTabmanager::Get()->TryInvokeTab(FName(TEXT("TileMapEditorTab")));
		}))
	);
}

TSharedRef<SDockTab> FDev_GNCEditorModule::SpawnTileMapEditorTab(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.TabRole(NomadTab)
		.Label(FText::FromString(TEXT("TileMap Editor")))
		[
			SNew(STileMapEditorWindow)
		];
}
