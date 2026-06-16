using UnrealBuildTool;

public class UETerminalBridge : ModuleRules
{
	public UETerminalBridge(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});

		PrivateDependencyModuleNames.AddRange(new[]
		{
			"HTTPServer",
			"Json",
			"Projects",
			"DeveloperSettings",
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new[] {
				"UnrealEd",
				"EditorScriptingUtilities",
				"PythonScriptPlugin",
				"AssetTools",
				"AssetRegistry",
				"ContentBrowser",
				"LevelEditor",
				"InputCore",
				"Slate",
				"SlateCore",
				"SourceControl",
				"Projects",
			});
		}
	}
}
