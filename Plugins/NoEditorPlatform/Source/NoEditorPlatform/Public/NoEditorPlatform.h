// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "FRuntimeNoEditorTargetPlatform.h"
#include "ITargetPlatform.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "DeviceProfiles/DeviceProfile.h"

PRAGMA_DISABLE_OPTIMIZATION

class FNoEditorPlatformModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	bool ShouldCookPackageForPlatform(const UPackage* package, const ITargetPlatform* platform)
	{
		return false;
	};


	ITargetPlatform* GetWIndowsNoEditorPlftform(){
//#if !WITH_EDITOR
		if (Plftform)
		{
			return Plftform;
		}
		else
		{
			//FCoreUObjectDelegates::ShouldCookPackageForPlatform.BindRaw(this,&FNoEditorPlatformModule::ShouldCookPackageForPlatform);
			Plftform = new FRunTimeNoEditorTargetPlatform();
			const UDeviceProfile* TextureLODSettingsObj = UDeviceProfileManager::Get(true).FindProfile(FString("WindowsNoEditor"));
			Plftform->RegisterTextureLODSettings(TextureLODSettingsObj);
			return Plftform;
		}
//#else
	return nullptr;
//#endif
	};
private:
	FRunTimeNoEditorTargetPlatform* Plftform;
};



PRAGMA_ENABLE_OPTIMIZATION