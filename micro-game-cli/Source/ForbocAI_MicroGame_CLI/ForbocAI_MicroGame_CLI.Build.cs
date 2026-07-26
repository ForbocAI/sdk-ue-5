using UnrealBuildTool;

public class ForbocAI_MicroGame_CLI : ModuleRules
{
	public ForbocAI_MicroGame_CLI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"ForbocAI_SDK",
			"HTTP",
			"Json",
			"JsonUtilities"
		});
	}
}
