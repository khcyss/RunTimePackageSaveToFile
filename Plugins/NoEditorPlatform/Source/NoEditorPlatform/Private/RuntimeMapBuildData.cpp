// Fill out your copyright notice in the Description page of Project Settings.


#include "RuntimeMapBuildData.h"
#include "BufferArchive.h"
#include "IImageWrapper.h"
#include "ModuleManager.h"
#include "IImageWrapperModule.h"
#include "Engine/Texture.h"
#include "Engine/Texture2DDynamic.h"
#include "LightMap.h"
#include "ShadowMap.h"
#include "Engine/MapBuildDataRegistry.h"


void URuntimeMapBuildData::SaveData()
{
	FBufferArchive ToBinary;

	TArray<uint8> Data;
	for (TPair<FGuid,FMeshMapBuildData> Each : MeshBuildData)
	{
		FGuid guid = Each.Key;
		FLightMap* LightMap = Each.Value.LightMap;
		FLightMap2D* LightMap2d = dynamic_cast<FLightMap2D*>(LightMap);
		//if (LightMap2d)
		//{
			/*TArray<UTexture2D*> AllRefTextures;
			LightMap2d->GetReferencedTextures(AllRefTextures);
			for (auto EachTexture : AllRefTextures)
			{
				
			}*/
		//}
	}
}

TArray<uint8> URuntimeMapBuildData::TextureToArray(class UTexture* Texture)
{
	TArray<uint8> OutData;
	EImageFormat ImageFormat = EImageFormat::PNG;
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	TSharedPtr<IImageWrapper>ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	Texture->Source.GetMipData(OutData, 0);
	FTexturePlatformData*& PlatformData = *Texture->GetRunningPlatformData();
	float Width = PlatformData->SizeX;
	float Height = PlatformData->SizeY;
	int Depth = Texture->Source.GetFormat() == ETextureSourceFormat::TSF_RGBA16 ? 16 : 8;
	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetRaw(OutData.GetData(), OutData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, Depth))
	{
		OutData = ImageWrapperPtr->GetCompressed(100);
	}
	return OutData;
}