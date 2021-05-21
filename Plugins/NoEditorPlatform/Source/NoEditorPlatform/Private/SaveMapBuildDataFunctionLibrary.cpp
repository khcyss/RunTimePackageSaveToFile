// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveMapBuildDataFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Parse.h"
#include "UObjectGlobals.h"
#include "Package.h"
#include "CoreGlobals.h"
#include "TargetPlatform/Public/Interfaces/ITargetPlatformModule.h"
#include "ModuleManager.h"
#include "TargetPlatform/Public/Interfaces/ITargetPlatform.h"
#include "LinkerLoad.h"
#include "NoEditorPlatform.h"
#include "CoreMisc.h"
#include "ITargetPlatformManagerModule.h"
#include "IPluginManager.h"
#include "Engine/Texture2D.h"
#include "Engine/MapBuildDataRegistry.h"



PRAGMA_DISABLE_OPTIMIZATION

void URuntimeSaveMapBuildData::SaveMapDataToFile(FString PackageName, FString FinalPackageSavePath)
{
	/*FSaveErrorOutputDevice SaveErrors;*/
	//GEngine->Exec(NULL, *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath), SaveErrors);
	//GEngine->Exec(NULL, *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath));


	FString Str = FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath);


	UPackage* Pkg;

	FString TempFname;
	if (FParse::Value(*Str, TEXT("FILE="), TempFname) && ParseObject<UPackage>(*Str, TEXT("Package="), Pkg, NULL))
	{
		bool bSilent = false;
		bool bAutosaving = false;
		bool bKeepDirty = false;
		FParse::Bool(*Str, TEXT("SILENT="), bSilent);
		FParse::Bool(*Str, TEXT("AUTOSAVING="), bAutosaving);
		FParse::Bool(*Str, TEXT("KEEPDIRTY="), bKeepDirty);

		Pkg->MarkPackageDirty();

		// If we have a loader for the package, unload it to prevent conflicts if we are resaving to the same filename
		FLinkerLoad* Loader = FLinkerLoad::FindExistingLinkerForPackage(Pkg);
		if (Loader)
		{
			Loader->GetLinker();
		}
		SaveMapBuildata(Pkg, FinalPackageSavePath);
		//Pkg->LinkerLoad->DetachAllBulkData(true);
		CookPackage(Pkg, FinalPackageSavePath);
		
	}
}



bool URuntimeSaveMapBuildData::CookPackage(UPackage* Package, const FString& SavePath)
{
	bool bSuccessed = false;
	const bool bSaveConcurrent = FParse::Param(FCommandLine::Get(), TEXT("ConcurrentSave"));
	bool bUnversioned = false;
	uint32 SaveFlags = SAVE_KeepGUID | SAVE_Async | SAVE_ComputeHash | (bUnversioned ? SAVE_Unversioned : 0);
	EObjectFlags CookedFlags = RF_Public;
	if (Cast<UWorld>(Package))
	{
		CookedFlags = RF_NoFlags;
	}
	if (bSaveConcurrent)
	{
		SaveFlags |= SAVE_Concurrent;
	}

	FNoEditorPlatformModule* WindowsNoEditorPlatformModule = FModuleManager::LoadModulePtr<FNoEditorPlatformModule>("NoEditorPlatform");
	if (WindowsNoEditorPlatformModule)
	{
	if (Package->FileName.IsNone())
		return bSuccessed;

		struct FFilterEditorOnlyFlag
		{
			FFilterEditorOnlyFlag(UPackage* InPackage, ITargetPlatform* InPlatform)
			{
				Package = InPackage;
				Platform = InPlatform;
				if (!Platform->HasEditorOnlyData())
				{
					Package->SetPackageFlags(PKG_FilterEditorOnly);
				}
				else
				{
					Package->ClearPackageFlags(PKG_FilterEditorOnly);
				}
			}
			~FFilterEditorOnlyFlag()
			{
				if (!Platform->HasEditorOnlyData())
				{
					Package->ClearPackageFlags(PKG_FilterEditorOnly);
				}
			}
			UPackage* Package;
			ITargetPlatform* Platform;
		};
		ITargetPlatform* Platform = WindowsNoEditorPlatformModule->GetWIndowsNoEditorPlftform();
		FFilterEditorOnlyFlag SetPackageEditorOnlyFlag(Package, Platform);

		FString CookedSavePath = SavePath;
		// delete old cooked assets
		if (FPaths::FileExists(CookedSavePath))
		{
			IFileManager::Get().Delete(*CookedSavePath);
		}
		Package->FullyLoad();
		TArray<UObject*> ExportMap;
		GetObjectsWithOuter(Package, ExportMap);
		for (const auto& ExportObj : ExportMap)
		{
#if WITH_EDITOR
	
		ExportObj->BeginCacheForCookedPlatformData(Platform);
#else
		UTexture2D* Texture = Cast<UTexture2D>(ExportObj);
		if (Texture)
		{
			FTexturePlatformData*& TempTexturePlatformData = *Texture->GetRunningPlatformData();
			UE_LOG(LogTemp,Warning,TEXT("Texture mips: %d"), TempTexturePlatformData->SizeX);
		}
		
#endif	
		}


		GIsCookerLoadingPackage = true;
		//UE_LOG(LogTemp, Warning, TEXT("Cook Assets:%s save to %s"), *Package->GetName(), *CookedSavePath);
		FSavePackageResultStruct Result = UPackage::Save(Package, nullptr, CookedFlags, *CookedSavePath, GError, nullptr, false, false, SaveFlags, WindowsNoEditorPlatformModule->GetWIndowsNoEditorPlftform(), FDateTime::MinValue(), false, /*DiffMap*/ nullptr); //GEditor->Save(Package, nullptr, CookedFlags, *CookedSavePath,GError, nullptr, false, false, SaveFlags, Platform,FDateTime::MinValue(), false, /*DiffMap*/ nullptr);
		GIsCookerLoadingPackage = false;
		bSuccessed = Result == ESavePackageResult::Success;
	}
	return bSuccessed;
}




void URuntimeSaveMapBuildData::SaveMapBuildata(UPackage* MapBuildDataPackage, const FString& FileSave)
{
	TArray<UObject*> ExportMap;
	GetObjectsWithOuter(MapBuildDataPackage, ExportMap);
	UMapBuildDataRegistry* MapBuilData = nullptr;
	TArray<UObject*> ExportTexture;
	for (auto EachObject: ExportMap)
	{
		if (EachObject->IsA(UMapBuildDataRegistry::StaticClass()))
		{
			MapBuilData = Cast<UMapBuildDataRegistry>(EachObject);
		}
		else if (EachObject->IsA(UTexture::StaticClass()))
		{
			ExportTexture.Add(EachObject);
		}
	}
}

PRAGMA_ENABLE_OPTIMIZATION