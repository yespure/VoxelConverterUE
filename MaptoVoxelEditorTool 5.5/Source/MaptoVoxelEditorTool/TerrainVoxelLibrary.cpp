#include "TerrainVoxelLibrary.h"

#include "Engine/Engine.h"
#include "ImageUtils.h"
#include "DrawDebugHelpers.h"

void UTerrainVoxelLibrary::TestCppBridge()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            -1,
            3.0f,
            FColor::Green,
            TEXT("C++ bridge works")
        );
    }
}

void UTerrainVoxelLibrary::TestDepthRenderTarget(
    UTextureRenderTarget2D* RenderTarget
)
{
    if (!RenderTarget)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                TEXT("Render Target is NULL")
            );
        }

        return;
    }

    const int32 Width = RenderTarget->SizeX;
    const int32 Height = RenderTarget->SizeY;

    if (GEngine)
    {
        const FString Message = FString::Printf(
            TEXT("Render Target: %d x %d"),
            Width,
            Height
        );

        GEngine->AddOnScreenDebugMessage(
            -1,
            5.0f,
            FColor::Green,
            Message
        );
    }
}

void UTerrainVoxelLibrary::ReadCenterDepth(
    UTextureRenderTarget2D* RenderTarget
)
{
    if (!RenderTarget)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                TEXT("ReadCenterDepth: Render Target is NULL")
            );
        }

        return;
    }

    const int32 Width = RenderTarget->SizeX;
    const int32 Height = RenderTarget->SizeY;

    if (Width <= 0 || Height <= 0)
    {
        return;
    }

    const int32 CenterX = Width / 2;
    const int32 CenterY = Height / 2;

    const FIntRect ReadRect(
        CenterX,
        CenterY,
        CenterX + 1,
        CenterY + 1
    );

    FImage Image;

    const bool bSuccess =
        FImageUtils::GetRenderTargetImage(
            RenderTarget,
            Image,
            ReadRect
        );

    if (!bSuccess || Image.SizeX != 1 || Image.SizeY != 1)
    {
        return;
    }

    const FLinearColor Pixel =
        Image.AsRGBA32F()[0];

    const float DepthValue =
        Pixel.R;

    if (GEngine)
    {
        const FString Message =
            FString::Printf(
                TEXT("Depth Pixel (%d, %d) = %.4f"),
                CenterX,
                CenterY,
                DepthValue
            );

        GEngine->AddOnScreenDebugMessage(
            -1,
            8.0f,
            FColor::Yellow,
            Message
        );
    }
}

void UTerrainVoxelLibrary::DrawCenterDepthPoint(
    USceneCaptureComponent2D* SceneCapture,
    UTextureRenderTarget2D* RenderTarget
)
{
    if (!SceneCapture || !RenderTarget)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                TEXT("DrawCenterDepthPoint: Invalid input")
            );
        }

        return;
    }

    const int32 Width =
        RenderTarget->SizeX;

    const int32 Height =
        RenderTarget->SizeY;

    if (Width <= 0 || Height <= 0)
    {
        return;
    }

    const int32 CenterX =
        Width / 2;

    const int32 CenterY =
        Height / 2;

    const FIntRect ReadRect(
        CenterX,
        CenterY,
        CenterX + 1,
        CenterY + 1
    );

    FImage Image;

    const bool bSuccess =
        FImageUtils::GetRenderTargetImage(
            RenderTarget,
            Image,
            ReadRect
        );

    if (!bSuccess ||
        Image.SizeX != 1 ||
        Image.SizeY != 1)
    {
        return;
    }

    const FLinearColor Pixel =
        Image.AsRGBA32F()[0];

    const float Depth =
        Pixel.R;

    if (!FMath::IsFinite(Depth) ||
        Depth <= 0.0f)
    {
        return;
    }

    const FVector CameraLocation =
        SceneCapture->GetComponentLocation();

    const FVector CameraForward =
        SceneCapture->GetForwardVector();

    const FVector WorldPosition =
        CameraLocation +
        CameraForward * Depth;

    DrawDebugPoint(
        SceneCapture->GetWorld(),
        WorldPosition,
        20.0f,
        FColor::Red,
        false,
        10.0f
    );

    if (GEngine)
    {
        const FString Message =
            FString::Printf(
                TEXT(
                    "World Point: X=%.2f Y=%.2f Z=%.2f | Depth=%.2f"
                ),
                WorldPosition.X,
                WorldPosition.Y,
                WorldPosition.Z,
                Depth
            );

        GEngine->AddOnScreenDebugMessage(
            -1,
            10.0f,
            FColor::Cyan,
            Message
        );
    }
}

void UTerrainVoxelLibrary::DrawDepthPointCloud(
    USceneCaptureComponent2D* SceneCapture,
    UTextureRenderTarget2D* DepthRenderTarget,
    UTextureRenderTarget2D* ColorRenderTarget,
    UTextureRenderTarget2D* NormalRenderTarget,
    UTextureRenderTarget2D* MaterialRenderTarget,
    int32 PixelStep,
    float PointSize,
    float Duration,
    float MaximumDepth,
    TArray<FVoxelSamplePoint>& OutPoints
)
{
    OutPoints.Empty();

    if (!SceneCapture ||
        !DepthRenderTarget)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                TEXT("PointCloud: Invalid input")
            );
        }

        return;
    }

    PixelStep =
        FMath::Max(PixelStep, 1);

    const int32 Width =
        DepthRenderTarget->SizeX;

    const int32 Height =
        DepthRenderTarget->SizeY;

    if (Width <= 0 ||
        Height <= 0)
    {
        return;
    }

    const float OrthoWidth =
        SceneCapture->OrthoWidth;

    if (OrthoWidth <= 0.0f)
    {
        return;
    }

    UWorld* World =
        SceneCapture->GetWorld();

    if (!World)
    {
        return;
    }

    FTextureRenderTargetResource*
        DepthResource =
        DepthRenderTarget->
        GameThread_GetRenderTargetResource();

    if (!DepthResource)
    {
        return;
    }

    TArray<FLinearColor> DepthPixels;

    FReadSurfaceDataFlags ReadFlags;
    ReadFlags.SetLinearToGamma(false);

    const bool bDepthSuccess =
        DepthResource->ReadLinearColorPixels(
            DepthPixels,
            ReadFlags
        );

    if (!bDepthSuccess)
    {
        return;
    }

    if (DepthPixels.Num() != Width * Height)
    {
        return;
    }

    TArray<FLinearColor> ColorPixels;
    TArray<FLinearColor> NormalPixels;
    TArray<FLinearColor> MaterialPixels;

    if (ColorRenderTarget)
    {
        FTextureRenderTargetResource*
            ColorResource =
            ColorRenderTarget->
            GameThread_GetRenderTargetResource();

        if (ColorResource)
        {
            ColorResource->ReadLinearColorPixels(
                ColorPixels,
                ReadFlags
            );
        }
    }

    if (NormalRenderTarget)
    {
        FTextureRenderTargetResource*
            NormalResource =
            NormalRenderTarget->
            GameThread_GetRenderTargetResource();

        if (NormalResource)
        {
            NormalResource->ReadLinearColorPixels(
                NormalPixels,
                ReadFlags
            );
        }
    }

    if (MaterialRenderTarget)
    {
        FTextureRenderTargetResource*
            MaterialResource =
            MaterialRenderTarget->
            GameThread_GetRenderTargetResource();

        if (MaterialResource)
        {
            MaterialResource->ReadLinearColorPixels(
                MaterialPixels,
                ReadFlags
            );
        }
    }

    const FVector CameraLocation =
        SceneCapture->GetComponentLocation();

    const FVector CameraForward =
        SceneCapture->GetForwardVector();

    const FVector CameraRight =
        SceneCapture->GetRightVector();

    const FVector CameraUp =
        SceneCapture->GetUpVector();

    const float WorldUnitsPerPixelX =
        OrthoWidth /
        static_cast<float>(Width);

    const float WorldUnitsPerPixelY =
        OrthoWidth /
        static_cast<float>(Height);

    OutPoints.Reserve(
        ((Width + PixelStep - 1) / PixelStep) *
        ((Height + PixelStep - 1) / PixelStep)
    );

    const float HalfWidth =
        static_cast<float>(Width) * 0.5f;

    const float HalfHeight =
        static_cast<float>(Height) * 0.5f;

    for (int32 Y = 0;
        Y < Height;
        Y += PixelStep)
    {
        for (int32 X = 0;
            X < Width;
            X += PixelStep)
        {
            const int32 PixelIndex =
                Y * Width + X;

            if (!DepthPixels.IsValidIndex(PixelIndex))
            {
                continue;
            }

            const float Depth =
                DepthPixels[PixelIndex].R;

            if (!FMath::IsFinite(Depth) ||
                Depth <= 0.0f ||
                Depth > MaximumDepth)
            {
                continue;
            }

            const float OffsetX =
                (
                    static_cast<float>(X) +
                    0.5f -
                    HalfWidth
                    ) *
                WorldUnitsPerPixelX;

            const float OffsetY =
                (
                    HalfHeight -
                    (
                        static_cast<float>(Y) +
                        0.5f
                        )
                    ) *
                WorldUnitsPerPixelY;

            const FVector WorldPosition =
                CameraLocation +
                CameraForward * Depth +
                CameraRight * OffsetX +
                CameraUp * OffsetY;

            FVoxelSamplePoint Point;

            Point.Position =
                WorldPosition;

            Point.Color =
                FLinearColor::White;

            if (ColorPixels.IsValidIndex(PixelIndex))
            {
                Point.Color =
                    ColorPixels[PixelIndex];
            }

            Point.Normal =
                FVector::UpVector;

            if (NormalPixels.IsValidIndex(PixelIndex))
            {
                const FLinearColor NormalColor =
                    NormalPixels[PixelIndex];

                Point.Normal =
                    FVector(
                        NormalColor.R * 2.0f - 1.0f,
                        NormalColor.G * 2.0f - 1.0f,
                        NormalColor.B * 2.0f - 1.0f
                    ).GetSafeNormal();
            }

            Point.MaterialID =
                0;

            if (MaterialPixels.IsValidIndex(PixelIndex))
            {
                Point.MaterialID =
                    FMath::RoundToInt(
                        MaterialPixels[PixelIndex].R *
                        255.0f
                    );
            }

            OutPoints.Add(Point);

            DrawDebugPoint(
                World,
                WorldPosition,
                PointSize,
                FColor::Green,
                false,
                Duration
            );
        }
    }

    if (GEngine)
    {
        const FString Message =
            FString::Printf(
                TEXT(
                    "Point Cloud SUCCESS | Points=%d | RT=%dx%d | Step=%d"
                ),
                OutPoints.Num(),
                Width,
                Height,
                PixelStep
            );

        GEngine->AddOnScreenDebugMessage(
            -1,
            10.0f,
            FColor::Green,
            Message
        );
    }
}

void UTerrainVoxelLibrary::ScanAllCameras(
    AActor* CaptureRig,
    UTextureRenderTarget2D* DepthRenderTarget,
    UTextureRenderTarget2D* ColorRenderTarget,
    UTextureRenderTarget2D* NormalRenderTarget,
    UTextureRenderTarget2D* MaterialRenderTarget,
    int32 PixelStep,
    float PointSize,
    float Duration,
    float MaximumDepth,
    TArray<FVoxelSamplePoint>& OutPoints
)
{
    OutPoints.Empty();

    if (!CaptureRig)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                TEXT("ScanAllCameras: CaptureRig is NULL")
            );
        }

        return;
    }

    if (!DepthRenderTarget)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                TEXT("ScanAllCameras: Depth RT is NULL")
            );
        }

        return;
    }

    TArray<USceneCaptureComponent2D*> CaptureComponents;

    CaptureRig->GetComponents<USceneCaptureComponent2D>(
        CaptureComponents
    );

    if (CaptureComponents.Num() == 0)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                TEXT("ScanAllCameras: No cameras found")
            );
        }

        return;
    }

    int32 ValidCameraCount = 0;

    for (USceneCaptureComponent2D* Capture :
        CaptureComponents)
    {
        if (!Capture)
        {
            continue;
        }

        Capture->TextureTarget =
            DepthRenderTarget;

        Capture->CaptureScene();

        TArray<FVoxelSamplePoint>
            CameraPoints;

        DrawDepthPointCloud(
            Capture,
            DepthRenderTarget,
            ColorRenderTarget,
            NormalRenderTarget,
            MaterialRenderTarget,
            PixelStep,
            PointSize,
            Duration,
            MaximumDepth,
            CameraPoints
        );

        OutPoints.Append(
            CameraPoints
        );

        ValidCameraCount++;
    }

    if (GEngine)
    {
        const FString Message =
            FString::Printf(
                TEXT(
                    "Multi Camera Scan SUCCESS | Cameras=%d | Points=%d"
                ),
                ValidCameraCount,
                OutPoints.Num()
            );

        GEngine->AddOnScreenDebugMessage(
            -1,
            10.0f,
            FColor::Green,
            Message
        );
    }
}

void UTerrainVoxelLibrary::DeduplicateVoxelSamplePoints(
    const TArray<FVoxelSamplePoint>& InPoints,
    float VoxelSize,
    TArray<FVoxelSamplePoint>& OutPoints
)
{
    OutPoints.Empty();

    if (VoxelSize <= 0.0f)
    {
        return;
    }

    TSet<FIntVector> OccupiedVoxels;
    OutPoints.Reserve(InPoints.Num());

    for (const FVoxelSamplePoint& Point : InPoints) {
        const FVector& Position = Point.Position;
        const FIntVector VoxelCoordinate(
            FMath::FloorToInt(Position.X / VoxelSize),
            FMath::FloorToInt(Position.Y / VoxelSize),
            FMath::FloorToInt(Position.Z / VoxelSize)
        );

        if (OccupiedVoxels.Contains(VoxelCoordinate))
        {
            continue;
        }

        OccupiedVoxels.Add(VoxelCoordinate);
        OutPoints.Add(Point);
    }

    if (GEngine)
    {
        const FString Message = FString::Printf(
            TEXT("Deduplicate SUCCESS | Input=%d | Output=%d | Removed=%d"),
            InPoints.Num(),
            OutPoints.Num(),
            InPoints.Num() - OutPoints.Num()
        );

        GEngine->AddOnScreenDebugMessage(
            -1,
            10.0f,
            FColor::Green,
            Message
        );
    }
}

void UTerrainVoxelLibrary::VoxelizeSamplePoints(
    const TArray<FVoxelSamplePoint>& InputPoints,
    float VoxelSize,
    TArray<FVoxelSamplePoint>& OutVoxelPoints
)
{
    OutVoxelPoints.Empty();


    if (VoxelSize <= 0.0f)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Red,
                TEXT("VoxelizeSamplePoints: VoxelSize <= 0")
            );
        }

        return;
    }

    if (InputPoints.Num() == 0)
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(
                -1,
                5.0f,
                FColor::Yellow,
                TEXT("VoxelizeSamplePoints: InputPoints is empty")
            );
        }

        return;
    }



    TMap<FIntVector, FVoxelSamplePoint> VoxelMap;

    VoxelMap.Reserve(InputPoints.Num());



    for (const FVoxelSamplePoint& InputPoint : InputPoints)
    {
        if (InputPoint.Position.ContainsNaN())
        {
            continue;
        }


        const int32 GridX =
            FMath::FloorToInt(
                InputPoint.Position.X / VoxelSize
            );

        const int32 GridY =
            FMath::FloorToInt(
                InputPoint.Position.Y / VoxelSize
            );

        const int32 GridZ =
            FMath::FloorToInt(
                InputPoint.Position.Z / VoxelSize
            );


        const FIntVector GridKey(
            GridX,
            GridY,
            GridZ
        );

        FVoxelSamplePoint VoxelPoint;


        VoxelPoint.Position =
            FVector(
                GridX * VoxelSize,
                GridY * VoxelSize,
                GridZ * VoxelSize
            );

        // 保留扫描得到的数据
        VoxelPoint.Color =
            InputPoint.Color;

        VoxelPoint.Normal =
            InputPoint.Normal;

        VoxelPoint.MaterialID =
            InputPoint.MaterialID;


        if (!VoxelMap.Contains(GridKey))
        {
            VoxelMap.Add(
                GridKey,
                VoxelPoint
            );
        }
    }


    OutVoxelPoints.Reserve(
        VoxelMap.Num()
    );

    for (const TPair<FIntVector, FVoxelSamplePoint>& Pair :
        VoxelMap)
    {
        OutVoxelPoints.Add(
            Pair.Value
        );
    }

    if (GEngine)
    {
        const FString Message =
            FString::Printf(
                TEXT(
                    "Voxelize SUCCESS | "
                    "Input=%d | Unique Voxels=%d | "
                    "VoxelSize=%.2f"
                ),
                InputPoints.Num(),
                OutVoxelPoints.Num(),
                VoxelSize
            );

        GEngine->AddOnScreenDebugMessage(
            -1,
            10.0f,
            FColor::Green,
            Message
        );
    }
}