using UnrealBuildTool;

public class sim : ModuleRules
{
    public sim(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "OnlineSubsystem",
            "OnlineSubsystemUtils",
            "OnlineSubsystemSteam",
            "OnlineSubsystemEOS",
            "UMG",
            "Slate",
            "SlateCore",
            "PhysicsCore",
            "AIModule",
            "NavigationSystem",
            "GameplayTasks",
            "Niagara",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "DeveloperSettings",
        });
    }
}
