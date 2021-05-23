// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveMapBuildDataFunctionLibrary.h"
#include "Engine/Engine.h"
#include "Parse.h"
#include "UObjectGlobals.h"
#include "Package.h"
#include "CoreGlobals.h"
#include "TargetPlatform/Public/Interfaces/ITargetPlatformModule.h"
#include "ModuleManager.h"
#include "TargetPlatform/Public/Interfaces/ITargetPlatform.h"
#include "LinkerLoad.h"
#include "NoEditorPlatform.h"
#include "CoreMisc.h"
#include "ITargetPlatformManagerModule.h"
#include "IPluginManager.h"
#include "Engine/Texture2D.h"
#include "Engine/MapBuildDataRegistry.h"
#include "Kismet/GameplayStatics.h"
#include "IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "BufferArchive.h"
#include "FileHelper.h"
#include "MemoryReader.h"
#include "Engine/ShadowMapTexture2D.h"
#include "Engine/LightMapTexture2D.h"




PRAGMA_DISABLE_OPTIMIZATION

//
//void URuntimeSaveMapBuildDataFunctionLibrary::SaveMapDataToFile(FString PackageName, FString FinalPackageSavePath)
//{
//	/*FSaveErrorOutputDevice SaveErrors;*/
//	//GEngine->Exec(NULL, *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath), SaveErrors);
//	//GEngine->Exec(NULL, *FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath));
//
//
//	FString Str = FString::Printf(TEXT("OBJ SAVEPACKAGE PACKAGE=\"%s\" FILE=\"%s\" SILENT=true"), *PackageName, *FinalPackageSavePath);
//
//
//	UPackage* Pkg;
//
//	FString TempFname;
//	if (FParse::Value(*Str, TEXT("FILE="), TempFname) && ParseObject<UPackage>(*Str, TEXT("Package="), Pkg, NULL))
//	{
//		bool bSilent = false;
//		bool bAutosaving = false;
//		bool bKeepDirty = false;
//		FParse::Bool(*Str, TEXT("SILENT="), bSilent);
//		FParse::Bool(*Str, TEXT("AUTOSAVING="), bAutosaving);
//		FParse::Bool(*Str, TEXT("KEEPDIRTY="), bKeepDirty);
//
//		Pkg->MarkPackageDirty();
//
//		// If we have a loader for the package, unload it to prevent conflicts if we are resaving to the same filename
//		FLinkerLoad* Loader = FLinkerLoad::FindExistingLinkerForPackage(Pkg);
//		if (Loader)
//		{
//			Loader->GetLinker();
//		}
//		SaveMapBuildata(Pkg, FinalPackageSavePath);
//		//Pkg->LinkerLoad->DetachAllBulkData(true);
//		//CookPackage(Pkg, FinalPackageSavePath);
//		
//	}
//}
//
//
//
//bool URuntimeSaveMapBuildDataFunctionLibrary::CookPackage(UPackage* Package, const FString& SavePath)
//{
//	bool bSuccessed = false;
//	const bool bSaveConcurrent = FParse::Param(FCommandLine::Get(), TEXT("ConcurrentSave"));
//	bool bUnversioned = false;
//	uint32 SaveFlags = SAVE_KeepGUID | SAVE_Async | SAVE_ComputeHash | (bUnversioned ? SAVE_Unversioned : 0);
//	EObjectFlags CookedFlags = RF_Public;
//	if (Cast<UWorld>(Package))
//	{
//		CookedFlags = RF_NoFlags;
//	}
//	if (bSaveConcurrent)
//	{
//		SaveFlags |= SAVE_Concurrent;
//	}
//
//	FNoEditorPlatformModule* WindowsNoEditorPlatformModule = FModuleManager::LoadModulePtr<FNoEditorPlatformModule>("NoEditorPlatform");
//	if (WindowsNoEditorPlatformModule)
//	{
//	if (Package->FileName.IsNone())
//		return bSuccessed;
//
//		struct FFilterEditorOnlyFlag
//		{
//			FFilterEditorOnlyFlag(UPackage* InPackage, ITargetPlatform* InPlatform)
//			{
//				Package = InPackage;
//				Platform = InPlatform;
//				if (!Platform->HasEditorOnlyData())
//				{
//					Package->SetPackageFlags(PKG_FilterEditorOnly);
//				}
//				else
//				{
//					Package->ClearPackageFlags(PKG_FilterEditorOnly);
//				}
//			}
//			~FFilterEditorOnlyFlag()
//			{
//				if (!Platform->HasEditorOnlyData())
//				{
//					Package->ClearPackageFlags(PKG_FilterEditorOnly);
//				}
//			}
//			UPackage* Package;
//			ITargetPlatform* Platform;
//		};
//		ITargetPlatform* Platform = WindowsNoEditorPlatformModule->GetWIndowsNoEditorPlftform();
//		FFilterEditorOnlyFlag SetPackageEditorOnlyFlag(Package, Platform);
//
//		FString CookedSavePath = SavePath;
//		// delete old cooked assets
//		if (FPaths::FileExists(CookedSavePath))
//		{
//			IFileManager::Get().Delete(*CookedSavePath);
//		}
//		Package->FullyLoad();
//		TArray<UObject*> ExportMap;
//		GetObjectsWithOuter(Package, ExportMap);
//		for (const auto& ExportObj : ExportMap)
//		{
//#if WITH_EDITOR
//	
//		ExportObj->BeginCacheForCookedPlatformData(Platform);
//#else
//		UTexture2D* Texture = Cast<UTexture2D>(ExportObj);
//		if (Texture)
//		{
//			FTexturePlatformData*& TempTexturePlatformData = *Texture->GetRunningPlatformData();
//			UE_LOG(LogTemp,Warning,TEXT("Texture mips: %d"), TempTexturePlatformData->SizeX);
//		}
//		
//#endif	
//		}
//
//
//		GIsCookerLoadingPackage = true;
//		//UE_LOG(LogTemp, Warning, TEXT("Cook Assets:%s save to %s"), *Package->GetName(), *CookedSavePath);
//		FSavePackageResultStruct Result = UPackage::Save(Package, nullptr, CookedFlags, *CookedSavePath, GError, nullptr, false, false, SaveFlags, WindowsNoEditorPlatformModule->GetWIndowsNoEditorPlftform(), FDateTime::MinValue(), false, /*DiffMap*/ nullptr); //GEditor->Save(Package, nullptr, CookedFlags, *CookedSavePath,GError, nullptr, false, false, SaveFlags, Platform,FDateTime::MinValue(), false, /*DiffMap*/ nullptr);
//		GIsCookerLoadingPackage = false;
//		bSuccessed = Result == ESavePackageResult::Success;
//	}
//	return bSuccessed;
//}
//
//
//
//
//void URuntimeSaveMapBuildDataFunctionLibrary::SaveMapBuildata(UPackage* MapBuildDataPackage, const FString& FileSave)
//{
//	TArray<UObject*> ExportMap;
//	GetObjectsWithOuter(MapBuildDataPackage, ExportMap);
//	UMapBuildDataRegistry* MapBuilData = nullptr;
//	TArray<UObject*> ExportTexture;
//	for (auto EachObject: ExportMap)
//	{
//		if (EachObject->IsA(UMapBuildDataRegistry::StaticClass()))
//		{
//			MapBuilData = Cast<UMapBuildDataRegistry>(EachObject);
//		}
//		else if (EachObject->IsA(UTexture::StaticClass()))
//		{
//			ExportTexture.Add(EachObject);
//		}
//	}
//
//	Save(MapBuilData,FileSave);
//}
//
//
//
//void URuntimeSaveMapBuildDataFunctionLibrary::Save(UMapBuildDataRegistry* MapBuildDataReg, const FString& FileToSavePath)
//{
//	FBufferArchive ToBinary;
//	TArray<uint8> Data;
//	ToBinary << MapBuildDataReg->LevelLightingQuality;
//	int32 Size = MapBuildDataReg->GetAllMeshBuildData().Num();
//	ToBinary << Size;
//
//	//MushBuildData
//	for (TPair<FGuid, FMeshMapBuildData> Each : MapBuildDataReg->GetAllMeshBuildData())
//	{
//
//		FGuid guid = Each.Key;
//		ToBinary << guid;
//		FLightMap2D* LightMap2d = Each.Value.LightMap->GetLightMap2D();
//		if (LightMap2d)
//		{
//			TArray<ULightMapTexture2D*> AllRefTextures;
//			LightMap2d->GetReferencedTextures(AllRefTextures);
//			bool HasSkyOcclusionTexture = LightMap2d->GetSkyOcclusionTexture() ? true : false;
//			bool HasAOMaterialMaskTexture = LightMap2d->GetAOMaterialMaskTexture() ? true : false;
//			int32 TextureNum = AllRefTextures.Num();
//			ToBinary << TextureNum;
//			ToBinary << HasAOMaterialMaskTexture;
//			ToBinary << HasSkyOcclusionTexture;
//			ToBinary << Each.Value.IrrelevantLights;
//			ToBinary << LightMap2d->LightGuids;
//			for (uint32 CoefficientIndex = 0; CoefficientIndex < 4; CoefficientIndex++)
//			{
//				ToBinary << LightMap2d->ScaleVectors[CoefficientIndex];
//				ToBinary << LightMap2d->AddVectors[CoefficientIndex];
//			}
//			ToBinary << LightMap2d->CoordinateScale;
//			ToBinary << LightMap2d->CoordinateBias;
//			Each.Value.PerInstanceLightmapData.BulkSerialize(ToBinary);
//			for (ULightMapTexture2D* EachTexture : AllRefTextures)
//			{
//				uint32 LightmapFlags = EachTexture->LightmapFlags;
//				ToBinary << LightmapFlags;
//				TArray<uint8> Texturedata = TextureToArray(EachTexture);
//				ToBinary << Texturedata;
//			}
//		}
//		bool HasShadowMap = Each.Value.ShadowMap ? true : false;
//		ToBinary << HasShadowMap;
//		if (Each.Value.ShadowMap)
//		{
//			FShadowMap2D* ShadowMap = Each.Value.ShadowMap->GetShadowMap2D();
//			if (ShadowMap)
//			{
//				if (ShadowMap->GetTexture())
//				{
//					ToBinary << ShadowMap->LightGuids;
//					TArray<uint8> Texturedata = TextureToArray(ShadowMap->GetTexture());
//					ToBinary << Texturedata;
//				}
//			}
//		}
//	}
//
//	//偷个懒走回官方的序列化
//	MapBuildDataReg->RuntimeSaveLoadData(ToBinary);
//	FFileHelper::SaveArrayToFile(ToBinary, TEXT("C:/Users/Khcy/Desktop/SaveTestFile"));
//
//	ToBinary.FlushCache();
//	ToBinary.Empty();
//}
//
//void URuntimeSaveMapBuildDataFunctionLibrary::Load(class UMapBuildDataRegistry*& MapBuildDataReg, UWorld* CurrentWorld)
//{
//	TArray<uint8> BinaryArray;
//	if (FFileHelper::LoadFileToArray(BinaryArray, TEXT("C:/Users/Khcy/Desktop/SaveTestFile")))
//	{
//		FMemoryReader FromBinary = FMemoryReader(BinaryArray, true);
//		FromBinary.Seek(0);
//		FromBinary << MapBuildDataReg->LevelLightingQuality;
//		int32 Size;
//		FromBinary << Size;
//		TMap<FGuid, FMeshMapBuildData> MeshBuildData;
//		for (int i = 0; i < Size; i++)
//		{
//			FMeshMapBuildData MeshBuid = FMeshMapBuildData();
//			TRefCountPtr<FLightMap2D> LightMap = TRefCountPtr<FLightMap2D>(new FLightMap2D());
//			TRefCountPtr<FShadowMap2D> ShadowMap = TRefCountPtr<FShadowMap2D>(new FShadowMap2D());
//			FGuid guid;
//			int32 TextureNum;
//			bool HasSkyOcclusionTexture = false;
//			bool HasAOMaterialMaskTexture = false;
//			FromBinary << guid;
//			FromBinary << TextureNum;
//			FromBinary << HasAOMaterialMaskTexture;
//			FromBinary << HasSkyOcclusionTexture;
//			FromBinary << MeshBuid.IrrelevantLights;
//			FromBinary << LightMap->LightGuids;
//			for (uint32 CoefficientIndex = 0; CoefficientIndex < 4; CoefficientIndex++)
//			{
//				FromBinary << LightMap->ScaleVectors[CoefficientIndex];
//				FromBinary << LightMap->AddVectors[CoefficientIndex];
//			}
//			FromBinary << LightMap->CoordinateScale;
//			FromBinary << LightMap->CoordinateBias;
//			MeshBuid.PerInstanceLightmapData.BulkSerialize(FromBinary);
//			for (int j = 0; j < TextureNum; j++)
//			{
//				uint32 LightmapFlags;
//				FromBinary << LightmapFlags;
//				TArray<uint8> TextureFileData;
//				FromBinary << TextureFileData;
//				ULightMapTexture2D* Image;
//				float Hegiht;
//				float width;
//				LoadImagedataToTexture<ULightMapTexture2D>(MapBuildDataReg->GetOuter(),TextureFileData, Image, width, Hegiht);
//				Image->LightmapFlags = ELightMapFlags(LightmapFlags);
//				LightMap->Textures[j] = Image;
//			}
//			if(TextureNum > 0) 
//			{
//				MeshBuid.LightMap = LightMap;
//				LightMap->SetStatusUpdate(true);
//				LightMap->AddRef();
//			}else
//				MeshBuid.LightMap = NULL;
//			bool HasShadowMap = false;
//			FromBinary << HasShadowMap;
//			if (HasShadowMap)
//			{
//				MeshBuid.ShadowMap = ShadowMap;
//				ShadowMap->AddRef();
//			}else
//				MeshBuid.ShadowMap = NULL;
//			if (HasShadowMap)
//			{
//				FromBinary << ShadowMap->LightGuids;
//				TArray<uint8> TextureFileData;
//				FromBinary << TextureFileData;
//				UShadowMapTexture2D* Image;
//				float Hegiht;
//				float width;
//				LoadImagedataToTexture<UShadowMapTexture2D>(MapBuildDataReg->GetOuter(),TextureFileData, Image, width, Hegiht);
//				ShadowMap->Texture = Image;
//			}
//			MeshBuildData.Add(guid, MeshBuid);
//		}
//		//设置回去
//		MapBuildDataReg->SetAllMeshBuildData(MeshBuildData);
//		//走回官方的序列化
//		MapBuildDataReg->RuntimeSaveLoadData(FromBinary);
//		if (CurrentWorld)
//		{
//			CurrentWorld->GetCurrentLevel()->InitializeRenderingResources();
//			CurrentWorld->GetCurrentLevel()->SetLightingScenario(true);
//		}
//	}
//}
//
//TArray<uint8> URuntimeSaveMapBuildDataFunctionLibrary::TextureToArray(class UTexture* Texture)
//{
//	TArray<uint8> OutData;
//	EImageFormat ImageFormat = EImageFormat::PNG;
//	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
//	TSharedPtr<IImageWrapper>ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFormat);
//	Texture->Source.GetMipData(OutData, 0);
//	FTexturePlatformData*& PlatformData = *Texture->GetRunningPlatformData();
//	float Width = PlatformData->SizeX;
//	float Height = PlatformData->SizeY;
//	int Depth = Texture->Source.GetFormat() == ETextureSourceFormat::TSF_RGBA16 ? 16 : 8;
//	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetRaw(OutData.GetData(), OutData.GetAllocatedSize(), Width, Height, ERGBFormat::BGRA, Depth))
//	{
//		OutData = ImageWrapperPtr->GetCompressed(100);
//	}
//	return OutData;
//}
//
//void URuntimeSaveMapBuildDataFunctionLibrary::LoadMapData(const FString& FilePath, UObject* CurrentWorld)
//{
//	UMapBuildDataRegistry* TempLoad = CurrentWorld->GetWorld()->GetCurrentLevel()->GetOrCreateMapBuildData();
//	Load(TempLoad, CurrentWorld->GetWorld());
//	TempLoad->GetAllMeshBuildData();
//}
//
//
//
//
//template<class T>
//bool URuntimeSaveMapBuildDataFunctionLibrary::LoadImagedataToTexture(class UObject* Outer,TArray<uint8> FileData, T*& InTexture, float& Width, float& Height)
//{
//	const TArray<uint8>* ImageData = new TArray<uint8>();
//	EImageFormat ImageFormat = EImageFormat::PNG;
//	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>("ImageWrapper");
//	TSharedPtr<IImageWrapper>ImageWrapperPtr = ImageWrapperModule.CreateImageWrapper(ImageFormat);
//	if (ImageWrapperPtr.IsValid() && ImageWrapperPtr->SetCompressed(FileData.GetData(), FileData.GetAllocatedSize()))
//	{
//		//OutData 与格式无关的颜色数据
//		ImageWrapperPtr->GetRaw(ERGBFormat::BGRA, 8, ImageData);
//		Width = ImageWrapperPtr->GetWidth();
//		Height = ImageWrapperPtr->GetHeight();
//		int32 SizeX = Width;
//		int32 Sizey = Height;
//		if (SizeX > 0 && Sizey > 0 &&
//			(SizeX % GPixelFormats[PF_B8G8R8A8].BlockSizeX) == 0 &&
//			(Sizey % GPixelFormats[PF_B8G8R8A8].BlockSizeY) == 0)
//		{
//			InTexture = NewObject<T>(
//				Outer,
//				NAME_None,
//				RF_Transient
//				);
//
//			InTexture->PlatformData = new FTexturePlatformData();
//			InTexture->PlatformData->SizeX = Width;
//			InTexture->PlatformData->SizeY = Height;
//			InTexture->PlatformData->PixelFormat = PF_B8G8R8A8;
//
//			// Allocate first mipmap.
//			int32 NumBlocksX = SizeX / GPixelFormats[PF_B8G8R8A8].BlockSizeX;
//			int32 NumBlocksY = Sizey / GPixelFormats[PF_B8G8R8A8].BlockSizeY;
//			FTexture2DMipMap* Mip = new FTexture2DMipMap();
//			InTexture->PlatformData->Mips.Add(Mip);
//			Mip->SizeX = Width;
//			Mip->SizeY = Height;
//			Mip->BulkData.Lock(LOCK_READ_WRITE);
//			Mip->BulkData.Realloc(NumBlocksX * NumBlocksY * GPixelFormats[PF_B8G8R8A8].BlockBytes);
//			Mip->BulkData.Unlock();
//		}
//		if (InTexture)
//		{
//			void* TextureData = InTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
//			FMemory::Memcpy(TextureData, ImageData->GetData(), ImageData->Num());
//			InTexture->PlatformData->Mips[0].BulkData.Unlock();
//			InTexture->UpdateResource();
//
//			return true;
//		}
//	}
//	return false;
//}
//



PRAGMA_ENABLE_OPTIMIZATION