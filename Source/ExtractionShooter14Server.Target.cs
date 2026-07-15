using UnrealBuildTool;
using System.Collections.Generic;

public class ExtractionShooter14ServerTarget : TargetRules
{
    public ExtractionShooter14ServerTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Server;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
        ExtraModuleNames.Add("ExtractionShooter14");

        GlobalDefinitions.Add("UE_PROJECT_STEAMSHIPPINGID=480");
        GlobalDefinitions.Add("UE_PROJECT_STEAMPRODUCTNAME=\"spacewar\"");
        GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDIR=\"spacewar\"");
        GlobalDefinitions.Add("UE_PROJECT_STEAMGAMEDESC=\"Spacewar\"");
    }
}