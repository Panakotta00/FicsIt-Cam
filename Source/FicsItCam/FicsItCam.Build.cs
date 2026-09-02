// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;
using System;
using EpicGames.Core;

public class FicsItCam : ModuleRules
{
    public FicsItCam(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	    CppStandard = CppStandardVersion.Cpp20;

	    // Force WITH_MGPU to 0 for this module to match game binaries
	    PublicDefinitions.Add("WITH_MGPU=0");
	    PrivateDefinitions.Add("WITH_MGPU=0");

		PublicDependencyModuleNames.AddRange(new string[]
		{
            "Core", "CoreUObject",
            "Engine",
            "InputCore",
            "UMG",
            "Slate", "SlateCore",
            "InteractiveToolsFramework",
            "CinematicCamera",
            "RHI",
            "Renderer",
            "RenderCore",
            "ImageWrapper",
            "Niagara",
            "EnhancedInput",
            "WwiseSoundEngine",
            "HTTP",
		});
			
        PublicDependencyModuleNames.AddRange(new string[]
        {
	        "FactoryGame",
	        "SML"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
	        "AbstractInstance",
        });

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
	        var thirdPartyFolder = Path.Combine(ModuleDirectory, "../../ThirdParty");
	        PublicIncludePaths.Add(Path.Combine(thirdPartyFolder, "include"));

	        var platformName = Target.Platform.ToString();
	        var libraryFolder = Path.Combine(thirdPartyFolder, platformName);

	        PublicDelayLoadDLLs.Add("avcodec-60.dll");
	        PublicDelayLoadDLLs.Add("avformat-60.dll");
	        PublicDelayLoadDLLs.Add("swscale-7.dll");
	        PublicDelayLoadDLLs.Add("swresample-4.dll");
	        PublicDelayLoadDLLs.Add("avutil-58.dll");
	        RuntimeDependencies.Add("$(BinaryOutputDir)\\avcodec-60.dll", Path.Combine(libraryFolder, "avcodec-60.dll"));
	        RuntimeDependencies.Add("$(BinaryOutputDir)\\avformat-60.dll", Path.Combine(libraryFolder, "avformat-60.dll"));
	        RuntimeDependencies.Add("$(BinaryOutputDir)\\swscale-7.dll", Path.Combine(libraryFolder, "swscale-7.dll"));
	        RuntimeDependencies.Add("$(BinaryOutputDir)\\swresample-4.dll", Path.Combine(libraryFolder, "swresample-4.dll"));
	        RuntimeDependencies.Add("$(BinaryOutputDir)\\avutil-58.dll", Path.Combine(libraryFolder, "avutil-58.dll"));
	        PublicAdditionalLibraries.Add(Path.Combine(libraryFolder, "avcodec.lib"));
	        PublicAdditionalLibraries.Add(Path.Combine(libraryFolder, "avformat.lib"));
	        PublicAdditionalLibraries.Add(Path.Combine(libraryFolder, "swscale.lib"));
	        PublicAdditionalLibraries.Add(Path.Combine(libraryFolder, "swresample.lib"));
	        PublicAdditionalLibraries.Add(Path.Combine(libraryFolder, "avutil.lib"));
        }
    }
}
