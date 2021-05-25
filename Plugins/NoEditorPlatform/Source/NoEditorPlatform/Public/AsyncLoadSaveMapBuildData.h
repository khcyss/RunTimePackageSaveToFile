// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintAsyncActionBase.h"
#include "IImageWrapper.h"
#include "AsyncLoadSaveMapBuildData.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FLoadSaveTaskFinish,bool,Success);

/**
 * 
 */
UCLASS()
class NOEDITORPLATFORM_API UAsyncLoadSaveMapBuildData : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()
public:
	UAsyncLoadSaveMapBuildData();

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName= "AsyncLoadMapBuildData"))
		static UAsyncLoadSaveMapBuildData* AsyncLoadData(UObject* WorldContextObject, const FString& FileToSavePath);
	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true", WorldContext = "WorldContextObject", DisplayName = "AsnycSaveMapBuildData"))
		static UAsyncLoadSaveMapBuildData* AsyncSaveData(UObject* WorldContextObject, const FString& FileToSavePath);

	void Save(UObject* WorldContextObject, const FString& FileToSavePath);

	void Load(UObject* WorldContextObject, const FString& FileToSavePath);



	static TArray<uint8> TextureToArray(class UTexture* Texture);

	template<class T>
	static bool LoadImagedataToTexture(class UObject* outer, TArray<uint8> FileData,ERGBFormat format, T*& InTexture, float& Width, float& Height);

public:
	UPROPERTY(BlueprintAssignable)
		FLoadSaveTaskFinish LoadSaveTaskFinish;
};
