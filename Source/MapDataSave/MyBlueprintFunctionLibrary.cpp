// Fill out your copyright notice in the Description page of Project Settings.


#include "MyBlueprintFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Parse.h"
#include "UObjectGlobals.h"
#include "Package.h"
//#include "BusyCursor.h"
#include "CoreGlobals.h"
#include "TargetPlatform/Public/Interfaces/ITargetPlatformModule.h"
#include "ModuleManager.h"
#include "TargetPlatform/Public/Interfaces/ITargetPlatform.h"
#include "LinkerLoad.h"
#include "NoEditorPlatform.h"

//#if WITH_EDITOR

#include "CoreMisc.h"
#include "ITargetPlatformManagerModule.h"
#include "IPluginManager.h"
#include "Engine/Texture2D.h"
//#include "Editor/EditorEngine.h"
//#include "Editor.h"

//#endif


PRAGMA_DISABLE_OPTIMIZATION

void UMyBlueprintFunctionLibrary::TestSaveMapDataToFile(FString PackageName, FString FinalPackageSavePath)
{
	/*FSaveErrorOutputDevice SaveErrors;*/
	//GEngine->Exec(NULL, *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath), SaveErrors);
	//GEngine->Exec(NULL, *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath));


	FString Str = FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath);


	UPackage* Pkg;
	bool bWasSuccessful = true;

	FString TempFname;
	if (FParse::Value(*Str, TEXT("FILE="), TempFname) && ParseObject<UPackage>(*Str, TEXT("Package="), Pkg, NULL))
	{
		bool bSilent = false;
		bool bAutosaving = false;
		bool bKeepDirty = false;
		FParse::Bool(*Str, TEXT("SILENT="), bSilent);
		FParse::Bool(*Str, TEXT("AUTOSAVING="), bAutosaving);
		FParse::Bool(*Str, TEXT("KEEPDIRTY="), bKeepDirty);

		// Save the package.
		const bool bIsMapPackage = UWorld::FindWorldInPackage(Pkg) != nullptr;

		UObject* Base = nullptr;
	
	
		CookPackage(Pkg, FinalPackageSavePath);


		

		////int64 Size = Pkg->LinkerLoad->TotalSize();
		//Pkg->MarkPackageDirty();
		//Pkg->ClearPackageFlags(PKG_FilterEditorOnly);
		//FNoEditorPlatformModule* WindowsNoEditorPlatformModule = FModuleManager::LoadModulePtr<FNoEditorPlatformModule>("NoEditorPlatform");
		//if (WindowsNoEditorPlatformModule)
		//{
		//	UPackage::PreSavePackageEvent.Broadcast(Pkg);
		//	FSavePackageResultStruct Result = UPackage::Save(Pkg, Base, EObjectFlags::RF_Public | EObjectFlags::RF_Standalone, *FinalPackageSavePath, GLog, Pkg->LinkerLoad, false, bWarnOfLongFilename, SaveFlags, WindowsNoEditorPlatformModule->GetWIndowsNoEditorPlftform(), FDateTime::MinValue(), false, NULL);
		//	Result.CookedHash;
		//	UE_LOG(LogTemp, Warning, TEXT("File Size : %d"), Result.TotalFileSize);
		//}

		//const ITargetPlatform* TargetPlatform = new TGenericWindowsTargetPlatform<false, false, false>(); //TPM.FindTargetPlatform(TargetPlatformNameString);
		
		//if (TargetPlatform)
		//{
			
		//}

		

		
	}
}



bool UMyBlueprintFunctionLibrary::CookPackage(UPackage* Package, const FString& SavePath)
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


PRAGMA_ENABLE_OPTIMIZATION