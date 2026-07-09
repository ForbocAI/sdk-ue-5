using UnrealBuildTool;

public class ForbocAI_TestGame_CLI : ModuleRules
{
	public ForbocAI_TestGame_CLI(ReadOnlyTargetRules Target) : base(Target)
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
