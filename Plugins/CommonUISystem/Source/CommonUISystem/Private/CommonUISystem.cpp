// Copyright Epic Games, Inc. All Rights Reserved.

#include "CommonUISystem.h"

#define LOCTEXT_NAMESPACE "FCommonUISystemModule"

DEFINE_LOG_CATEGORY(LogCommonUISystem)

void FCommonUISystemModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FCommonUISystemModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FCommonUISystemModule, CommonUISystem)