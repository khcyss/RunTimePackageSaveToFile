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
		// Allow commandlets proceed without testing if we need to check out on assumption that they know what they are doing.
		/*if (Pkg == nullptr || (!IsRunningCommandlet() && (GUnrealEd == nullptr || !GUnrealEd->CanSavePackage(Pkg))))
		{
			return false;
		}*/

		//const FScopedBusyCursor BusyCursor;

		bool bSilent = false;
		bool bAutosaving = false;
		bool bKeepDirty = false;
		FParse::Bool(*Str, TEXT("SILENT="), bSilent);
		FParse::Bool(*Str, TEXT("AUTOSAVING="), bAutosaving);
		FParse::Bool(*Str, TEXT("KEEPDIRTY="), bKeepDirty);

		// Save the package.
		const bool bIsMapPackage = UWorld::FindWorldInPackage(Pkg) != nullptr;
		/*const FText SavingPackageText = (bIsMapPackage)
			? FText::Format(NSLOCTEXT("UnrealEd", "SavingMapf", "Saving map {0}"), FText::FromString(Pkg->GetName()))
			: FText::Format(NSLOCTEXT("UnrealEd", "SavingAssetf", "Saving asset {0}"), FText::FromString(Pkg->GetName()));*/

		//FScopedSlowTask SlowTask(100, SavingPackageText, !bSilent);

		uint32 SaveFlags = bAutosaving ? SAVE_FromAutosave : SAVE_None;
		if (bKeepDirty)
		{
			SaveFlags |= SAVE_KeepDirty;
		}

		const bool bWarnOfLongFilename = !bAutosaving;
		//bWasSuccessful = SavePackage(Pkg, NULL, RF_Standalone, *TempFname, &Ar, NULL, false, bWarnOfLongFilename, SaveFlags);
		ITargetPlatformModule& TargetModule = FModuleManager::LoadModuleChecked<ITargetPlatformModule>("WindowsNoEditorTargetPlatform");
		ITargetPlatform* Target = TargetModule.GetTargetPlatform();

		UPackage::Save(Pkg, nullptr, RF_Standalone, *FinalPackageSavePath,GLog, NULL, false, bWarnOfLongFilename, SaveFlags, Target, FDateTime::MinValue(), false, NULL);
	}
}
