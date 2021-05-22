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
#include "FileHelper.h"
#include "MemoryReader.h"


PRAGMA_DISABLE_OPTIMIZATION

void URuntimeMapBuildData::SaveData()
{
	FBufferArchive ToBinary;

	TArray<uint8> Data;
	int32 Size = MeshBuildData.Num();
	ToBinary << Size;
	for (TPair<FGuid,FMeshMapBuildData> Each : MeshBuildData)
	{
		
		FGuid guid = Each.Key;
		ToBinary<<guid;
		FLightMap2D* LightMap2d = Each.Value.LightMap->GetLightMap2D();
		if (LightMap2d)
		{
			TArray<UTexture2D*> AllRefTextures;
			LightMap2d->GetReferencedTextures(AllRefTextures);
			bool HasSkyOcclusionTexture = LightMap2d->GetSkyOcclusionTexture()? true:false;
			bool HasAOMaterialMaskTexture = LightMap2d->GetAOMaterialMaskTexture()? true :false;
			int32 TextureNum = AllRefTextures.Num();
			ToBinary<<TextureNum;
			ToBinary << HasAOMaterialMaskTexture;
			ToBinary << HasSkyOcclusionTexture;
			for (auto EachTexture : AllRefTextures)
			{
				FString Start="";
				ToBinary<<Start;
				TArray<uint8> Texturedata = TextureToArray(EachTexture);
				ToBinary<<Texturedata;
			}
		}
		bool HasShadowMap = Each.Value.ShadowMap? true : false;
		ToBinary << HasShadowMap;
		if (Each.Value.ShadowMap)
		{
			FShadowMap2D* ShadowMap = Each.Value.ShadowMap->GetShadowMap2D();
			if (ShadowMap)
			{
				if (ShadowMap->GetTexture())
				{
					TArray<uint8> Texturedata = TextureToArray(ShadowMap->GetTexture());
					ToBinary << Texturedata;
				}
			}
		}
		
	}
	FFileHelper::SaveArrayToFile(ToBinary,TEXT("C:/Users/Khcy/Desktop/SaveTestFile"));

	ToBinary.FlushCache();
	ToBinary.Empty();
}

void URuntimeMapBuildData::loadData()
{
	TArray<uint8> BinaryArray;
	if (FFileHelper::LoadFileToArray(BinaryArray, TEXT("C:/Users/Khcy/Desktop/SaveTestFile")))
	{
		FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
		FromBinary.Seek(0);
		int32 Size;
		FromBinary<<Size;
		for (int i = 0; i < Size; i++)
		{
			FMeshMapBuildData MeshBuid = FMeshMapBuildData();
			MeshBuid.LightMap = TRefCountPtr<FLightMap2D>();
			FGuid guid;
			int32 TextureNum;
			bool HasSkyOcclusionTexture = false;
			bool HasAOMaterialMaskTexture = false;
			FromBinary << guid;
			FromBinary << TextureNum;
			FromBinary << HasAOMaterialMaskTexture;
			FromBinary << HasSkyOcclusionTexture;
			for (int j=0; j<TextureNum;j++)
			{
				FString Start;
				FromBinary<<Start;
				TArray<uint8> TextureFileData;
				FromBinary<<TextureFileData;
				UTexture2D* Image;
				float Hegiht;
				float width;
				LoadImagedataToTexture(TextureFileData, Image,width,Hegiht);
			}
			bool HasShadowMap =false;
			FromBinary<<HasShadowMap;
			if (HasShadowMap)
			{
				TArray<uint8> TextureFileData;
				FromBinary << TextureFileData;
				TArray<uint8> TextureColorData;
				UTexture2D* Image;
				float Hegiht;
				float width;
				LoadImagedataToTexture(TextureFileData, Image, width, Hegiht);
			}
			MeshBuildData.Add(guid,MeshBuid);
		}
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



bool URuntimeMapBuildData::LoadImagedataToTexture(TArray<uint8> FileData, UTexture2D*& InTexture, float& Width, float& Height)
{
	const TArray<uint8> *ImageData = new TArray<uint8>();
	EImageFormat ImageFormat = EImageFormat::PNG;
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	TSharedPtr<IImageWrapper>ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetCompressed(FileData.GetData(), FileData.GetAllocatedSize()))
	{
		//OutData 与格式无关的颜色数据
		ImageWrapperPtr->GetRaw(ERGBFormat::BGRA, 8, ImageData);
		Width = ImageWrapperPtr->GetWidth();
		Height = ImageWrapperPtr->GetHeight();
		InTexture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
		if (InTexture)
		{
			void* TextureData = InTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, ImageData->GetData(), ImageData->Num());
			InTexture->PlatformData->Mips[0].BulkData.Unlock();
			InTexture->UpdateResource();

			return true;
		}
	}
	return false;
}


PRAGMA_ENABLE_OPTIMIZATION

