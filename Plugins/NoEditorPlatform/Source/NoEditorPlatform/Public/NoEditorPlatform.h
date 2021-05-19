// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "FRuntimeNoEditorTargetPlatform.h"
#include "ITargetPlatform.h"

class FNoEditorPlatformModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	
	ITargetPlatform* GetWIndowsNoEditorPlftform(){
		if (Plftform)
		{
			return Plftform;
		}
		else
		{
			Plftform = new FRunTimeNoEditorTargetPlatform();
			return Plftform;
		}
	};
private:
	FRunTimeNoEditorTargetPlatform* Plftform;
};
