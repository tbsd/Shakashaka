// Copyright Epic Games, Inc. All Rights Reserved.

#include "LpSolvePlugin.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FLpSolvePluginModule"


void FLpSolvePluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("LpSolvePlugin")->GetBaseDir();

	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/LpSolvePluginLibrary/Win64/lpsolve55.dll"));
	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
#else
    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/LpSolvePluginLibrary/Android/armeabi-v7a/liblpsolve55j_armeabi-v7a.so"));
	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
	if (!ExampleLibraryHandle) {
		LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/LpSolvePluginLibrary/Android/arm64-v8a/liblpsolve55j_arm64-v8a.so"));
		ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
	}
#endif // PLATFORM_WINDOWS


	if (ExampleLibraryHandle) {
		UE_LOG(LogTemp, Log, TEXT("lpsolve loaded successfully!"));
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("lpsolve failed to load!"));
	}

}

void FLpSolvePluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	// Free the dll handle
	if (ExampleLibraryHandle) {
		FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
		ExampleLibraryHandle = nullptr;
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLpSolvePluginModule, LpSolvePlugin)
