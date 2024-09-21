// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class Sfizz4Unreal : ModuleRules
{
	public Sfizz4Unreal(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.Add(Path.Combine(PluginDirectory, "Sfizz"));

		
		PrivateDefinitions.AddRange(new string[] {
                "SFIZZ_EXPORT_SYMBOLS",

            });


		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			
			 RuntimeDependencies.Add("$(BinaryOutputDir)/sfizz.dll", Path.Combine(PluginDirectory, "Sfizz", "sfizz.dll"));
			// PublicDelayLoadDLLs.Add("sfizz.dll");
			 PublicAdditionalLibraries.Add(Path.Combine(PluginDirectory, "Sfizz", "sfizz.lib"));

		}


		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"Synthesis",
				"AudioExtensions",
				"AudioMixer",
				"AudioMixerCore",
				"SignalProcessing",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
