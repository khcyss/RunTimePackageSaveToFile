// Fill out your copyright notice in the Description page of Project Settings.


#include "AsyncLoadSaveMapBuildData.h"
#include "Engine/MapBuildDataRegistry.h"
#include "BufferArchive.h"
#include "Engine/LightMapTexture2D.h"
#include "ModuleManager.h"
#include "IImageWrapperModule.h"
#include "FileHelper.h"
#include "MemoryReader.h"
#include "Async.h"
#include "Engine/ShadowMapTexture2D.h"
#include "Object.h"
#include "Engine/World.h"

PRAGMA_DISABLE_OPTIMIZATION


UAsyncLoadSaveMapBuildData::UAsyncLoadSaveMapBuildData()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AddToRoot();
	}
}

UAsyncLoadSaveMapBuildData* UAsyncLoadSaveMapBuildData::AsyncLoadData(UObject* WorldContextObject, const FString& FileToSavePath)
{
	UAsyncLoadSaveMapBuildData* AsyncObj = NewObject<UAsyncLoadSaveMapBuildData>();
	AsyncTask(ENamedThreads::GameThread,[=](){
	AsyncObj->Load(WorldContextObject,FileToSavePath);
	});
	return AsyncObj;
}

UAsyncLoadSaveMapBuildData* UAsyncLoadSaveMapBuildData::AsyncSaveData(UObject* WorldContextObject, const FString& FileToSavePath)
{
	UAsyncLoadSaveMapBuildData* AsyncObj = NewObject<UAsyncLoadSaveMapBuildData>();
	AsyncTask(ENamedThreads::GameThread, [=]() {
		AsyncObj->Save(WorldContextObject, FileToSavePath);
		});
	return AsyncObj;
}

void UAsyncLoadSaveMapBuildData::Save(UObject* WorldContextObject, const FString& FileToSavePath)
{
	UMapBuildDataRegistry* MapBuildDataReg = WorldContextObject->GetWorld()->GetCurrentLevel()->MapBuildData;
	FBufferArchive ToBinary;
	TArray<uint8> Data;
	ToBinary << MapBuildDataReg->LevelLightingQuality;
	int32 Size = MapBuildDataReg->GetAllMeshBuildData().Num();
	ToBinary << Size;

	//MushBuildData
	for (TPair<FGuid, FMeshMapBuildData> Each : MapBuildDataReg->GetAllMeshBuildData())
	{

		FGuid guid = Each.Key;
		ToBinary << guid;
		FLightMap2D* LightMap2d = Each.Value.LightMap->GetLightMap2D();
		if (LightMap2d)
		{
			TArray<ULightMapTexture2D*> AllRefTextures;
			LightMap2d->GetReferencedTextures(AllRefTextures);
			bool HasSkyOcclusionTexture = LightMap2d->GetSkyOcclusionTexture() ? true : false;
			bool HasAOMaterialMaskTexture = LightMap2d->GetAOMaterialMaskTexture() ? true : false;
			int32 TextureNum = AllRefTextures.Num();
			ToBinary << TextureNum;
			ToBinary << HasAOMaterialMaskTexture;
			ToBinary << HasSkyOcclusionTexture;
			ToBinary << Each.Value.IrrelevantLights;
			ToBinary << LightMap2d->LightGuids;
			for (uint32 CoefficientIndex = 0; CoefficientIndex < 4; CoefficientIndex++)
			{
				ToBinary << LightMap2d->ScaleVectors[CoefficientIndex];
				ToBinary << LightMap2d->AddVectors[CoefficientIndex];
			}
			ToBinary << LightMap2d->CoordinateScale;
			ToBinary << LightMap2d->CoordinateBias;
			Each.Value.PerInstanceLightmapData.BulkSerialize(ToBinary);
			for (ULightMapTexture2D* EachTexture : AllRefTextures)
			{
				uint32 LightmapFlags = EachTexture->LightmapFlags;
				ToBinary << LightmapFlags;
				int8 ExportFormat = EachTexture->Source.GetFormat() == ETextureSourceFormat::TSF_G8 ? (int8)ERGBFormat::Gray : (int8)ERGBFormat::BGRA;
				ToBinary << ExportFormat;
				int8 sourceFormat = (int8)EachTexture->Source.GetFormat();
				ToBinary << sourceFormat;
				TArray<uint8> Texturedata = TextureToArray(EachTexture);
				ToBinary << Texturedata;
			}
		}
		//bool HasShadowMap = Each.Value.ShadowMap ? true : false;
		bool HasShadowMap = false;
		if (Each.Value.ShadowMap)
		{
			FShadowMap2D* ShadowMap = Each.Value.ShadowMap->GetShadowMap2D();
			if (ShadowMap)
			{
				if (ShadowMap->GetTexture())
				{
					HasShadowMap = true;
					ToBinary << HasShadowMap;
					ToBinary << ShadowMap->LightGuids;
					int8 ExportFormat = ShadowMap->GetTexture()->Source.GetFormat() == ETextureSourceFormat::TSF_G8 ? (int8)ERGBFormat::Gray : (int8)ERGBFormat::BGRA;
					ToBinary << ExportFormat;
					int8 sourceFormat = (int8)ShadowMap->GetTexture()->Source.GetFormat();
					ToBinary << sourceFormat;
					ToBinary << ShadowMap->CoordinateScale << ShadowMap->CoordinateBias;
					for (int Channel = 0; Channel < ARRAY_COUNT(ShadowMap->bChannelValid); Channel++)
					{
						ToBinary << ShadowMap->bChannelValid[Channel];
					}
					ToBinary<< ShadowMap->InvUniformPenumbraSize;
					TArray<uint8> Texturedata = TextureToArray(ShadowMap->GetTexture());
					ToBinary << Texturedata;
					ToBinary << (Cast<UShadowMapTexture2D>(ShadowMap->GetTexture()))->ShadowmapFlags;
				}
				else ToBinary << HasShadowMap;
			}else ToBinary << HasShadowMap;
		}else ToBinary << HasShadowMap;
	}

	MapBuildDataReg->RuntimeSaveLoadData(ToBinary);
	if (FFileHelper::SaveArrayToFile(ToBinary, *FileToSavePath))
	{
		if (LoadSaveTaskFinish.IsBound()) LoadSaveTaskFinish.Broadcast(true);
	}
	else
	{
		if (LoadSaveTaskFinish.IsBound()) LoadSaveTaskFinish.Broadcast(false);
	}

	ToBinary.FlushCache();
	ToBinary.Empty();
	RemoveFromRoot();
}

void UAsyncLoadSaveMapBuildData::Load(UObject* WorldContextObject, const FString& FileToSavePath)
{
	UMapBuildDataRegistry* MapBuildDataReg = WorldContextObject->GetWorld()->GetCurrentLevel()->GetOrCreateMapBuildData();
	TSet<FGuid> KeepLight;
	WorldContextObject->GetWorld()->GetCurrentLevel()->HandleLegacyMapBuildData();
	MapBuildDataReg->InvalidateStaticLighting(WorldContextObject->GetWorld(),true, &KeepLight);//初始化时清空一次数据
	TArray<uint8> BinaryArray;
	if (FFileHelper::LoadFileToArray(BinaryArray, *FileToSavePath))
	{
		FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
		FromBinary.Seek(0);
		FromBinary << MapBuildDataReg->LevelLightingQuality;
		int32 Size;
		FromBinary << Size;
		TMap<FGuid, FMeshMapBuildData> MeshBuildData;
		for (int i = 0; i < Size; i++)
		{
			FMeshMapBuildData MeshBuid = FMeshMapBuildData();
			TRefCountPtr<FLightMap2D> LightMap = TRefCountPtr<FLightMap2D>(new FLightMap2D());
			TRefCountPtr<FShadowMap2D> ShadowMap = TRefCountPtr<FShadowMap2D>(new FShadowMap2D());
			FGuid guid;
			int32 TextureNum;
			bool HasSkyOcclusionTexture = false;
			bool HasAOMaterialMaskTexture = false;
			FromBinary << guid;
			FromBinary << TextureNum;
			FromBinary << HasAOMaterialMaskTexture;
			FromBinary << HasSkyOcclusionTexture;
			FromBinary << MeshBuid.IrrelevantLights;
			FromBinary << LightMap->LightGuids;
			for (uint32 CoefficientIndex = 0; CoefficientIndex < 4; CoefficientIndex++)
			{
				FromBinary << LightMap->ScaleVectors[CoefficientIndex];
				FromBinary << LightMap->AddVectors[CoefficientIndex];
			}
			FromBinary << LightMap->CoordinateScale;
			FromBinary << LightMap->CoordinateBias;
			MeshBuid.PerInstanceLightmapData.BulkSerialize(FromBinary);
			for (int j = 0; j < TextureNum; j++)
			{
				uint32 LightmapFlags;
				int8 Format;
				int8 SourceFormat;
				FromBinary << LightmapFlags;
				FromBinary << Format;
				FromBinary << SourceFormat;
				TArray<uint8> TextureFileData;
				FromBinary << TextureFileData;
				ULightMapTexture2D* Image;
				float Hegiht;
				float width;
				LoadImagedataToTexture<ULightMapTexture2D>(MapBuildDataReg->GetOuter(), TextureFileData, (ERGBFormat)Format,(ETextureSourceFormat)SourceFormat, Image, width, Hegiht);
				Image->LightmapFlags = ELightMapFlags(LightmapFlags);
				if (HasSkyOcclusionTexture && j == TextureNum - 2)
				{
					LightMap->SkyOcclusionTexture = Image;
				}
				else if (HasAOMaterialMaskTexture && j == TextureNum - 1)
				{
					LightMap->AOMaterialMaskTexture = Image;
				}
				else
				{
					LightMap->Textures[j] = Image;
				}
			}
			if (TextureNum > 0)
			{
				MeshBuid.LightMap = LightMap;
				LightMap->SetStatusUpdate(true);
				LightMap->AddRef();
			}
			else
				MeshBuid.LightMap = NULL;
			bool HasShadowMap = false;
			FromBinary << HasShadowMap;
			if (HasShadowMap)
			{
				MeshBuid.ShadowMap = ShadowMap;
				ShadowMap->AddRef();
			}
			else
				MeshBuid.ShadowMap = NULL;
			if (HasShadowMap)
			{
				int8 Format;
				int8 SourceFormat;
				FVector2D CoordinateScale, CoordinateBias;
				FromBinary << ShadowMap->LightGuids;
				FromBinary << Format;
				FromBinary << SourceFormat;
				FromBinary << ShadowMap->CoordinateScale << ShadowMap->CoordinateBias;
				//FromBinary << CoordinateScale << CoordinateBias;
				for (int Channel = 0; Channel < ARRAY_COUNT(ShadowMap->bChannelValid); Channel++)
				{
					FromBinary << ShadowMap->bChannelValid[Channel];
				}
				FromBinary << ShadowMap->InvUniformPenumbraSize;
				TArray<uint8> TextureFileData;
				FromBinary << TextureFileData;
				UShadowMapTexture2D* Image = NULL;
				float Hegiht;
				float width;
				LoadImagedataToTexture<UShadowMapTexture2D>(MapBuildDataReg->GetOuter(), TextureFileData,(ERGBFormat)Format,(ETextureSourceFormat)SourceFormat, Image, width, Hegiht);
				Image->LODGroup = TEXTUREGROUP_Shadowmap;
				Image->SRGB = false;
				Image->MipGenSettings = TMGS_LeaveExistingMips;
				Image->CompressionNone = true;
				Image->UpdateResource();
				FromBinary << Image->ShadowmapFlags;
				ShadowMap->Texture = Image;
				ShadowMap->SetStatusUpdate(true);
			}
			MeshBuildData.Add(guid, MeshBuid);
		}
		//设置回去
		MapBuildDataReg->SetAllMeshBuildData(MeshBuildData);
		//走回官方的序列化
		MapBuildDataReg->RuntimeSaveLoadData(FromBinary);
		if (WorldContextObject)
		{
			WorldContextObject->GetWorld()->GetCurrentLevel()->InitializeRenderingResources();
			WorldContextObject->GetWorld()->GetCurrentLevel()->SetLightingScenario(true);
			if (LoadSaveTaskFinish.IsBound()) LoadSaveTaskFinish.Broadcast(true);
		}
		else
		{
			if (LoadSaveTaskFinish.IsBound()) LoadSaveTaskFinish.Broadcast(false);
		}
	}
	else
	{
		if (LoadSaveTaskFinish.IsBound()) LoadSaveTaskFinish.Broadcast(false);
	}
	RemoveFromRoot();
}

TArray<uint8> UAsyncLoadSaveMapBuildData::TextureToArray(class UTexture* Texture)
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
	const ERGBFormat ExportFormat = Texture->Source.GetFormat() == ETextureSourceFormat::TSF_G8 ? ERGBFormat::Gray : ERGBFormat::BGRA;
	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetRaw(OutData.GetData(), OutData.GetAllocatedSize(), Width, Height, ExportFormat, Depth))
	{
		OutData = ImageWrapperPtr->GetCompressed(100);
	}
	return OutData;
}




template<class T>
bool UAsyncLoadSaveMapBuildData::LoadImagedataToTexture(class UObject* outer, TArray<uint8> FileData, enum ERGBFormat format, ETextureSourceFormat SourceFormat, T*& InTexture, float& Width, float& Height)
{
	const TArray<uint8>* ImageData = new TArray<uint8>();
	EImageFormat ImageFormat = EImageFormat::PNG;
	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
	TSharedPtr<IImageWrapper>ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFormat);
	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetCompressed(FileData.GetData(), FileData.GetAllocatedSize()))
	{
		//OutData 与格式无关的颜色数据
		ImageWrapperPtr->GetRaw(format, 8, ImageData);
		Width = ImageWrapperPtr->GetWidth();
		Height = ImageWrapperPtr->GetHeight();
		int32 SizeX = Width;
		int32 Sizey = Height;
		if (SizeX > 0 && Sizey > 0 &&
			(SizeX % GPixelFormats[PF_B8G8R8A8].BlockSizeX) == 0 &&
			(Sizey % GPixelFormats[PF_B8G8R8A8].BlockSizeY) == 0)
		{
			InTexture = NewObject<T>(
				outer,
				NAME_None,
				RF_Transient
				);

			InTexture->PlatformData = new FTexturePlatformData();
			InTexture->PlatformData->SizeX = Width;
			InTexture->PlatformData->SizeY = Height;
			InTexture->PlatformData->NumSlices = 1;
			InTexture->PlatformData->PixelFormat = PF_B8G8R8A8;
			(Cast<UTexture2D>(InTexture))->Source.Init(Width, Height, 1, 1, SourceFormat, ImageData->GetData());
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
