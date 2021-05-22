// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/MapBuildDataRegistry.h"
#include "RuntimeMapBuildData.generated.h"

/**
 * 
 */
UCLASS()
class NOEDITORPLATFORM_API URuntimeMapBuildData : public UObject
{
	GENERATED_BODY()
public:
	void SaveData();

	TArray<uint8> TextureToArray(class UTexture* Texture);

	TMap<FGuid, FMeshMapBuildData> MeshBuildData;
	//TMap<FGuid, FPrecomputedLightVolumeData*> LevelPrecomputedLightVolumeBuildData;
	//TMap<FGuid, FPrecomputedVolumetricLightmapData*> LevelPrecomputedVolumetricLightmapBuildData;
	//TMap<FGuid, FLightComponentMapBuildData> LightBuildData;
	//TMap<FGuid, FReflectionCaptureMapBuildData> ReflectionCaptureBuildData;
};
