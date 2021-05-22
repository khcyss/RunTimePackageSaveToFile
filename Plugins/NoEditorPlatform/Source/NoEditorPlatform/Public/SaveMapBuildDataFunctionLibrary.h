// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveMapBuildDataFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class NOEDITORPLATFORM_API URuntimeSaveMapBuildData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
		static void SaveMapDataToFile(FString PackageName, FString FinalPackageSavePath);


	static bool CookPackage(UPackage* package, const FString& SavePath);


	static void SaveMapBuildata(UPackage* MapBuildDataPackage,const FString& FileSave);



	UFUNCTION(BlueprintCallable)
	static void LoadMapData(const FString& FilePath);

};
