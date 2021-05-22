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
#include "Engine/ShadowMapTexture2D.h"


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
			TArray<ULightMapTexture2D*> AllRefTextures;
			LightMap2d->GetReferencedTextures(AllRefTextures);
			bool HasSkyOcclusionTexture = LightMap2d->GetSkyOcclusionTexture()? true:false;
			bool HasAOMaterialMaskTexture = LightMap2d->GetAOMaterialMaskTexture()? true :false;
			int32 TextureNum = AllRefTextures.Num();
			ToBinary<<TextureNum;
			ToBinary << HasAOMaterialMaskTexture;
			ToBinary << HasSkyOcclusionTexture;
			for (ULightMapTexture2D* EachTexture : AllRefTextures)
			{
				uint32 LightmapFlags = EachTexture->LightmapFlags;
				ToBinary<< LightmapFlags;
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
	FFileHelper::SaveArrayToFile(ToBinary,TEXT("C:/Users/Administrator/Desktop/SaveTestFile"));

	ToBinary.FlushCache();
	ToBinary.Empty();
}

void URuntimeMapBuildData::loadData()
{
	TArray<uint8> BinaryArray;
	UMapBuildDataRegistry* MapBuildDataRegis = NewObject<UMapBuildDataRegistry>();
	if (FFileHelper::LoadFileToArray(BinaryArray, TEXT("C:/Users/Administrator/Desktop/SaveTestFile")))
	{
		FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
		FromBinary.Seek(0);
		int32 Size;
		FromBinary<<Size;
		for (int i = 0; i < Size; i++)
		{
			FMeshMapBuildData MeshBuid = FMeshMapBuildData();
			TRefCountPtr<FLightMap2D> LightMap = TRefCountPtr<FLightMap2D>(new FLightMap2D());
			TRefCountPtr<FShadowMap2D> ShadowMap = TRefCountPtr<FShadowMap2D>(new FShadowMap2D());
			MeshBuid.LightMap = LightMap;
			MeshBuid.ShadowMap = ShadowMap;
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
				uint32 LightmapFlags;
				FromBinary<< LightmapFlags;
				TArray<uint8> TextureFileData;
				FromBinary<<TextureFileData;
				ULightMapTexture2D* Image;
				float Hegiht;
				float width;
				LoadImagedataToTexture<ULightMapTexture2D>(TextureFileData, Image,width,Hegiht);
				Image->LightmapFlags = ELightMapFlags(LightmapFlags);
				LightMap->Textures[j] = Image;
			}
			bool HasShadowMap =false;
			FromBinary<<HasShadowMap;
			if (HasShadowMap)
			{
				TArray<FGuid> LightGuids;
				FromBinary << LightGuids;
				TArray<uint8> TextureFileData;
				FromBinary << TextureFileData;
				TArray<uint8> TextureColorData;
				UShadowMapTexture2D* Image;
				float Hegiht;
				float width;
				LoadImagedataToTexture<UShadowMapTexture2D>(TextureFileData, Image, width, Hegiht);
				ShadowMap->Texture = Image;
			}
			MeshBuildData.Add(guid,MeshBuid);
		}
		MapBuildDataRegis->SetAllMeshBuildData(MeshBuildData);
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


template<class T>
bool URuntimeMapBuildData::LoadImagedataToTexture(TArray<uint8> FileData, T*& InTexture, float& Width, float& Height)
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
		int32 SizeX = Width;
		int32 Sizey = Height;
		if (SizeX > 0 && Sizey > 0 &&
			(SizeX % GPixelFormats[PF_B8G8R8A8].BlockSizeX) == 0 &&
			(Sizey % GPixelFormats[PF_B8G8R8A8].BlockSizeY) == 0)
		{
			InTexture = NewObject<T>(
				GetTransientPackage(),
				NAME_None,
				RF_Transient
				);

			InTexture->PlatformData = new FTexturePlatformData();
			InTexture->PlatformData->SizeX = Width;
			InTexture->PlatformData->SizeY = Height;
			InTexture->PlatformData->PixelFormat = PF_B8G8R8A8;

			// Allocate first mipmap.
			int32 NumBlocksX = SizeX / GPixelFormats[PF_B8G8R8A8].BlockSizeX;
			int32 NumBlocksY = Sizey / GPixelFormats[PF_B8G8R8A8].BlockSizeY;
			FTexture2DMipMap* Mip = new FTexture2DMipMap();
			InTexture->PlatformData->Mips.Add(Mip);
			Mip->SizeX = Width;
			Mip->SizeY = Height;
			Mip->BulkData.Lock(LOCK_READ_WRITE);
			Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[PF_B8G8R8A8].BlockBytes);
			Mip->BulkData.Unlock();
		}
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

