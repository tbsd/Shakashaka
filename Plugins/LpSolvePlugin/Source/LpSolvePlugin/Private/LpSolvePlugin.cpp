// Copyright Epic Games, Inc. All Rights Reserved.

#include "LpSolvePlugin.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#include <fstream>

#define LOCTEXT_NAMESPACE "FLpSolvePluginModule"


void FLpSolvePluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 1"));
	// Get the base directory of this plugin
	FString BaseDir = IPluginManager::Get().FindPlugin("LpSolvePlugin")->GetBaseDir();

		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 2"));
	//std::ofstream o{ "/storage/emulated/0/com.android.meh.config/shakashaka.log", std::ios_base::app };
	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	//o << "HEHMDA 1" << std::endl;
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/LpSolvePluginLibrary/Win64/lpsolve55.dll"));
	//o << "HEHMDA 2" << std::endl;
	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 3"));
	//o << "HEHMDA 3" << std::endl;
#else
		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 4"));
	//o << "HEHMDA 4" << std::endl;
    LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/LpSolvePluginLibrary/Android/armeabi-v7a/liblpsolve55j_armeabi-v7a.so"));
	//o << "HEHMDA 5" << std::endl;
		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 5"));
	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
	//o << "HEHMDA 6" << std::endl;
	if (!ExampleLibraryHandle) {
	//o << "HEHMDA 7" << std::endl;
		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 6"));
		LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/LpSolvePluginLibrary/Android/arm64-v8a/liblpsolve55j_arm64-v8a.so"));
	//o << "HEHMDA 8" << std::endl;
		ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;
	//o << "HEHMDA 9" << std::endl;
		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 7"));
	}
#endif // PLATFORM_WINDOWS


	//o << "HEHMDA 10" << std::endl;
	if (ExampleLibraryHandle) {
	//o << "HEHMDA 11" << std::endl;
		UE_LOG(LogTemp, Log, TEXT("lpsolve loaded successfully!"));
		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 8"));
	}
	else {
	//o << "HEHMDA 12" << std::endl;
		UE_LOG(LogTemp, Warning, TEXT("lpsolve failed to load!"));
		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 9"));
	}

		UE_LOG(LogTemp, Log, TEXT("HEHMDA a 10"));
	//o << "HEHMDA 13" << std::endl;
	//o.close();
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
