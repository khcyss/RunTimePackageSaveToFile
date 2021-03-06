// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class MapDataSave : ModuleRules
{
	public MapDataSave(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" , "NoEditorPlatform", "SandboxFile", });


        PrivateDependencyModuleNames.AddRange(new string[] {  });


        //if (Target.bCompileAgainstEngine)
        //{
        PrivateDependencyModuleNames.AddRange(new string[] {
                "RHI",
                "Engine",
            "TargetPlatform" ,"DesktopPlatform","RenderCore","Projects",
          }
        );

        if(Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }


        PrivateIncludePathModuleNames.Add("TextureCompressor");
        PrivateIncludePathModuleNames.Add("TargetPlatform");

        //}

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
    }
}
