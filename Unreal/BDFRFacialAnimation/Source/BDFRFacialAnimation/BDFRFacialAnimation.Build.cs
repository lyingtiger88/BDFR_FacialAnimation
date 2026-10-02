using UnrealBuildTool;

public class BDFRFacialAnimation : ModuleRules
{
    public BDFRFacialAnimation(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "LiveLinkInterface",
                "Sockets",
                "Networking"
            }
        );
    }
}
