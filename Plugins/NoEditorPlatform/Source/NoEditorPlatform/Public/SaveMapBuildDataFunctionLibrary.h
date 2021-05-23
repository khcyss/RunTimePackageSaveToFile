// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveMapBuildDataFunctionLibrary.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FLoadSaveMapDataFinish,bool,Success);


/**
 * 
 */
UCLASS()
class NOEDITORPLATFORM_API URuntimeSaveMapBuildDataFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
/*
	UFUNCTION(BlueprintCallable)
		static void SaveMapDataToFile(FString PackageName, FString FinalPackageSavePath);

	UFUNCTION(BlueprintCallable)
		static void LoadMapData(const FString& FilePath, UObject* CurrentWorld);

	static void Save(class UMapBuildDataRegistry* MapBuildDataReg, const FString& FileToSavePath);

	static void Load(class UMapBuildDataRegistry*& MapBuildDataReg, UWorld* CurrentWorldToApply);


	static TArray<uint8> TextureToArray(class UTexture* Texture);

	template<class T>
	static bool LoadImagedataToTexture(class UObject* outer,TArray<uint8> FileData, T*& InTexture, float& Width, float& Height);



	//过期函数（SavePackage版本(也可以用不过会比较麻烦)）
	static bool CookPackage(UPackage* package, const FString& SavePath);

	static void SaveMapBuildata(UPackage* MapBuildDataPackage, const FString& FileSave);
	*/
};
