// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using System;
using UnrealBuildTool;

public class LpSolvePluginLibrary : ModuleRules
{
	public LpSolvePluginLibrary(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "lpsolve55.lib"));

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("lpsolve55.dll");

			// Ensure that the DLL is staged along with the executable
			RuntimeDependencies.Add("$(PluginDir)/Binaries/ThirdParty/LpSolvePluginLibrary/Win64/lpsolve55.dll");
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {

			AdditionalPropertiesForReceipt.Add("LpSolvePluginLibrary", Path.Combine(ModuleDirectory, "Android", "My_APL.xml"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "armeabi-v7a", "liblpsolve55j_armeabi-v7a.so"));
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "Android", "arm64-v8a", "liblpsolve55j_arm64-v8a.so"));
		}
        //else if (Target.Platform == UnrealTargetPlatform.Linux)
		//{
			//string ExampleSoPath = Path.Combine("$(PluginDir)", "Binaries", "ThirdParty", "LpSolvePluginLibrary", "Linux", "x86_64-unknown-linux-gnu", "libExampleLibrary.so");
		//	PublicAdditionalLibraries.Add(ExampleSoPath);
		//	PublicDelayLoadDLLs.Add(ExampleSoPath);
		//	RuntimeDependencies.Add(ExampleSoPath);
		//}
	}
}
