// Copyright GNC Project. All Rights Reserved.

using UnrealBuildTool;

public class Dev_GNCEditor : ModuleRules
{
	public Dev_GNCEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Slate",
			"SlateCore",
			"Dev_GNC"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"UnrealEd",
			"ToolMenus",
			"EditorStyle",
			"AssetTools",
			"ContentBrowserData"
		});

		PublicIncludePaths.AddRange(new string[] {
			"Dev_GNCEditor",
			"Dev_GNCEditor/CardGame/TileMapEditor"
		});
	}
}
