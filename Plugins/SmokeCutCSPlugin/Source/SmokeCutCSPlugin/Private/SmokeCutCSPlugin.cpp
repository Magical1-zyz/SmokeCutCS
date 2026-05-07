// Copyright Epic Games, Inc. All Rights Reserved.

#include "SmokeCutCSPlugin.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FSmokeCutCSPluginModule"

void FSmokeCutCSPluginModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("SmokeCutCSPlugin"));
	if (Plugin.IsValid())
	{
		const FString ShaderDir = FPaths::Combine(Plugin->GetBaseDir(), TEXT("Shaders"));
		AddShaderSourceDirectoryMapping(TEXT("/Plugin/SmokeCutCSPlugin"), ShaderDir);
	}
}

void FSmokeCutCSPluginModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSmokeCutCSPluginModule, SmokeCutCSPlugin)