#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Components/SceneCaptureComponent2D.h"
#include "TerrainVoxelLibrary.generated.h"

USTRUCT(BlueprintType)
struct FVoxelSamplePoint
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FVector Position;

    UPROPERTY(BlueprintReadWrite)
    FLinearColor Color;

    UPROPERTY(BlueprintReadWrite)
    FVector Normal;

    UPROPERTY(BlueprintReadWrite)
    int32 MaterialID;
};

UCLASS()
class MAPTOVOXELEDITORTOOL_API UTerrainVoxelLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION(BlueprintCallable, Category = "Terrain Voxel")
    static void TestCppBridge();

    UFUNCTION(BlueprintCallable, Category = "Terrain Voxel")
    static void TestDepthRenderTarget(
        UTextureRenderTarget2D* RenderTarget
    );

    UFUNCTION(BlueprintCallable, Category = "Terrain Voxel")
    static void ReadCenterDepth(
        UTextureRenderTarget2D* RenderTarget
    );

    UFUNCTION(BlueprintCallable, Category = "Terrain Voxel")
    static void DrawCenterDepthPoint(
        USceneCaptureComponent2D* SceneCapture,
        UTextureRenderTarget2D* RenderTarget
    );

    UFUNCTION(BlueprintCallable, Category = "Terrain Voxel")
    static void DrawDepthPointCloud(
        USceneCaptureComponent2D* SceneCapture,
        UTextureRenderTarget2D* DepthRenderTarget,
        UTextureRenderTarget2D* ColorRenderTarget,
        UTextureRenderTarget2D* NormalRenderTarget,
        UTextureRenderTarget2D* MaterialRenderTarget,
        int32 PixelStep,
        float PointSize,
        float Duration,
        TArray<FVoxelSamplePoint>& OutPoints
    );

    UFUNCTION(BlueprintCallable, Category = "Terrain Voxel")
    static void ScanAllCameras(
        AActor* CaptureRig,
        UTextureRenderTarget2D* DepthRenderTarget,
        UTextureRenderTarget2D* ColorRenderTarget,
        UTextureRenderTarget2D* NormalRenderTarget,
        UTextureRenderTarget2D* MaterialRenderTarget,
        int32 PixelStep,
        float PointSize,
        float Duration,
        TArray<FVoxelSamplePoint>& OutPoints
    );

    UFUNCTION(BlueprintCallable, Category = "Terrain Voxel")
    static void DeduplicateVoxelSamplePoints(
        const TArray<FVoxelSamplePoint>& InPoints,
        float VoxelSize,
        TArray<FVoxelSamplePoint>& OutPoints
    );

    UFUNCTION(BlueprintCallable, Category = "Terrain Voxel")
    static void VoxelizeSamplePoints(
        const TArray<FVoxelSamplePoint>& InputPoints,
        float VoxelSize,
        TArray<FVoxelSamplePoint>& OutVoxelPoints
    );
};