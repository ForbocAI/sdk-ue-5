using UnrealBuildTool;
using System.Collections.Generic;

public class ForbocAI_SDKTarget : TargetRules
{
	public ForbocAI_SDKTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;

		ExtraModuleNames.AddRange( new string[] { "ForbocAI_SDK" } );
	}
}
