using UnrealBuildTool;
using System.Collections.Generic;

public class FracturedStarsServerTarget : TargetRules
{
    public FracturedStarsServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;

        ExtraModuleNames.Add("FracturedStars");
    }
}