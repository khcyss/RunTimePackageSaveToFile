// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "NoEditorPlatform.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"


#define LOCTEXT_NAMESPACE "FNoEditorPlatformModule"

void FNoEditorPlatformModule::StartupModule()
{
	Plftform = nullptr;
}

void FNoEditorPlatformModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	
}

//ITargetPlatform* FNoEditorPlatformModule::GetWIndowsNoEditorPlftform()
//{
//	
//}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNoEditorPlatformModule, NoEditorPlatform)
