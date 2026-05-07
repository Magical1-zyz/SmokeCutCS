#include "SmokeCutShaderLibrary.h"

#include "DataDrivenShaderPlatformInfo.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GlobalShader.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "RHICommandList.h"
#include "ShaderParameterStruct.h"
#include "UnrealClient.h"

class FSmokeCutSimpleFillCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FSmokeCutSimpleFillCS);
    SHADER_USE_PARAMETER_STRUCT(FSmokeCutSimpleFillCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(FVector4f, FillColor)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputTexture)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

class FSmokeCutDrawCircleCS : public FGlobalShader
{
    public:
    DECLARE_GLOBAL_SHADER(FSmokeCutDrawCircleCS);
    SHADER_USE_PARAMETER_STRUCT(FSmokeCutDrawCircleCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(FVector2f, CenterUV)
        SHADER_PARAMETER(float, Radius)
        SHADER_PARAMETER(float, EdgeSoftness)
        SHADER_PARAMETER(float, Strength)
        SHADER_PARAMETER(FIntPoint, TextureSize)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputTexture)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

class FSmokeCutRecoverMaskCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FSmokeCutRecoverMaskCS);
    SHADER_USE_PARAMETER_STRUCT(FSmokeCutRecoverMaskCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(float, DeltaTime)
        SHADER_PARAMETER(float, RecoveryRate)
        SHADER_PARAMETER(FIntPoint, TextureSize)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputTexture)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

class FSmokeCutDrawCapsuleCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FSmokeCutDrawCapsuleCS);
    SHADER_USE_PARAMETER_STRUCT(FSmokeCutDrawCapsuleCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(FVector2f, CenterUV)
        SHADER_PARAMETER(FVector2f, DirectionUV)
        SHADER_PARAMETER(float, HalfLength)
        SHADER_PARAMETER(float, Radius)
        SHADER_PARAMETER(float, EdgeSoftness)
        SHADER_PARAMETER(float, Strength)
        SHADER_PARAMETER(FIntPoint, TextureSize)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputTexture)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

class FSmokeCutDiffuseMaskCS : public FGlobalShader
{
public:
    DECLARE_GLOBAL_SHADER(FSmokeCutDiffuseMaskCS);
    SHADER_USE_PARAMETER_STRUCT(FSmokeCutDiffuseMaskCS, FGlobalShader);

    BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
        SHADER_PARAMETER(float, DeltaTime)
        SHADER_PARAMETER(float, DiffuseStrength)
        SHADER_PARAMETER(FIntPoint, TextureSize)
        SHADER_PARAMETER_RDG_TEXTURE(Texture2D, InputTexture)
        SHADER_PARAMETER_SAMPLER(SamplerState, InputSampler)
        SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D, OutputTexture)
    END_SHADER_PARAMETER_STRUCT()

    static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
    {
        return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
    }
};

IMPLEMENT_GLOBAL_SHADER(
    FSmokeCutSimpleFillCS,
    "/Plugin/SmokeCutCSPlugin/Private/SimpleFillCS.usf",
    "MainCS",
    SF_Compute
);

IMPLEMENT_GLOBAL_SHADER(
    FSmokeCutDrawCircleCS,
    "/Plugin/SmokeCutCSPlugin/Private/DrawCircleCS.usf",
    "MainCS",
    SF_Compute
);

IMPLEMENT_GLOBAL_SHADER(
    FSmokeCutRecoverMaskCS,
    "/Plugin/SmokeCutCSPlugin/Private/RecoverMaskCS.usf",
    "MainCS",
    SF_Compute
);

IMPLEMENT_GLOBAL_SHADER(
    FSmokeCutDrawCapsuleCS,
    "/Plugin/SmokeCutCSPlugin/Private/DrawCapsuleCS.usf",
    "MainCS",
    SF_Compute
);

IMPLEMENT_GLOBAL_SHADER(
    FSmokeCutDiffuseMaskCS,
    "/Plugin/SmokeCutCSPlugin/Private/DiffuseMaskCS.usf",
    "MainCS",
    SF_Compute
);

void USmokeCutShaderLibrary::DispatchSimpleFill(UTextureRenderTarget2D* RenderTarget, FLinearColor FillColor)
{
    if (!RenderTarget)
    {
        return;
    }

    FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
    if (!RenderTargetResource)
    {
        return;
    }

    const FLinearColor FillColorCopy = FillColor;

    ENQUEUE_RENDER_COMMAND(SmokeCutSimpleFillCommand)(
        [RenderTargetResource, FillColorCopy](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            FRDGTextureRef OutputTexture = RenderTargetResource->GetRenderTargetTexture(GraphBuilder);
            if (!OutputTexture)
            {
                GraphBuilder.Execute();
                return;
            }

            TShaderMapRef<FSmokeCutSimpleFillCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            FSmokeCutSimpleFillCS::FParameters* PassParameters =
                GraphBuilder.AllocParameters<FSmokeCutSimpleFillCS::FParameters>();

            PassParameters->FillColor = FVector4f(
                (float)FillColorCopy.R,
                (float)FillColorCopy.G,
                (float)FillColorCopy.B,
                (float)FillColorCopy.A
            );

            PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));

            const FIntPoint TextureSize = OutputTexture->Desc.Extent;
            const FIntVector GroupCount(
                FMath::DivideAndRoundUp(TextureSize.X, 8),
                FMath::DivideAndRoundUp(TextureSize.Y, 8),
                1
            );

            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("SmokeCutCS_SimpleFill"),
                ComputeShader,
                PassParameters,
                GroupCount
            );

            GraphBuilder.Execute();
        }
    );
}

void USmokeCutShaderLibrary::DispatchDrawCircle(
    UTextureRenderTarget2D* RenderTarget,
    FVector2D CenterUV,
    float Radius,
    float EdgeSoftness,
    float Strength
)
{
    if (!RenderTarget)
    {
        return;
    }

    FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
    if (!RenderTargetResource)
    {
        return;
    }

    const FVector2D CenterUVCopy = CenterUV;
    const float RadiusCopy = Radius;
    const float EdgeSoftnessCopy = EdgeSoftness;
    const float StrengthCopy = Strength;

    ENQUEUE_RENDER_COMMAND(SmokeCutDrawCircleCommand)(
        [RenderTargetResource, CenterUVCopy, RadiusCopy, EdgeSoftnessCopy, StrengthCopy](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            FRDGTextureRef OutputTexture = RenderTargetResource->GetRenderTargetTexture(GraphBuilder);
            if (!OutputTexture)
            {
                GraphBuilder.Execute();
                return;
            }

            TShaderMapRef<FSmokeCutDrawCircleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            FSmokeCutDrawCircleCS::FParameters* PassParameters =
                GraphBuilder.AllocParameters<FSmokeCutDrawCircleCS::FParameters>();

            const FIntPoint TextureSize = OutputTexture->Desc.Extent;

            PassParameters->CenterUV = FVector2f((float)CenterUVCopy.X, (float)CenterUVCopy.Y);
            PassParameters->Radius = RadiusCopy;
            PassParameters->EdgeSoftness = EdgeSoftnessCopy;
            PassParameters->Strength = StrengthCopy;
            PassParameters->TextureSize = TextureSize;
            PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));

            const FIntVector GroupCount(
                FMath::DivideAndRoundUp(TextureSize.X, 8),
                FMath::DivideAndRoundUp(TextureSize.Y, 8),
                1
            );

            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("SmokeCutCS_DrawCircle"),
                ComputeShader,
                PassParameters,
                GroupCount
            );

            GraphBuilder.Execute();
        }
    );
}

void USmokeCutShaderLibrary::DispatchRecoverMask(
    UTextureRenderTarget2D* RenderTarget,
    float DeltaTime,
    float RecoveryRate
)
{
    if (!RenderTarget)
    {
        return;
    }

    FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
    if (!RenderTargetResource)
    {
        return;
    }

    const float DeltaTimeCopy = DeltaTime;
    const float RecoveryRateCopy = RecoveryRate;

    ENQUEUE_RENDER_COMMAND(SmokeCutRecoverMaskCommand)(
        [RenderTargetResource, DeltaTimeCopy, RecoveryRateCopy](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            FRDGTextureRef OutputTexture = RenderTargetResource->GetRenderTargetTexture(GraphBuilder);
            if (!OutputTexture)
            {
                GraphBuilder.Execute();
                return;
            }

            TShaderMapRef<FSmokeCutRecoverMaskCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            FSmokeCutRecoverMaskCS::FParameters* PassParameters =
                GraphBuilder.AllocParameters<FSmokeCutRecoverMaskCS::FParameters>();

            const FIntPoint TextureSize = OutputTexture->Desc.Extent;

            PassParameters->DeltaTime = DeltaTimeCopy;
            PassParameters->RecoveryRate = RecoveryRateCopy;
            PassParameters->TextureSize = TextureSize;
            PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));

            const FIntVector GroupCount(
                FMath::DivideAndRoundUp(TextureSize.X, 8),
                FMath::DivideAndRoundUp(TextureSize.Y, 8),
                1
            );

            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("SmokeCutCS_RecoverMask"),
                ComputeShader,
                PassParameters,
                GroupCount
            );

            GraphBuilder.Execute();
        }
    );
}

void USmokeCutShaderLibrary::DispatchDrawCapsule(
    UTextureRenderTarget2D* RenderTarget,
    FVector2D CenterUV,
    FVector2D DirectionUV,
    float HalfLength,
    float Radius,
    float EdgeSoftness,
    float Strength
)
{
    if (!RenderTarget)
    {
        return;
    }

    FTextureRenderTargetResource* RenderTargetResource = RenderTarget->GameThread_GetRenderTargetResource();
    if (!RenderTargetResource)
    {
        return;
    }

    const FVector2D CenterUVCopy = CenterUV;
    const FVector2D DirectionUVCopy = DirectionUV;
    const float HalfLengthCopy = HalfLength;
    const float RadiusCopy = Radius;
    const float EdgeSoftnessCopy = EdgeSoftness;
    const float StrengthCopy = Strength;

    ENQUEUE_RENDER_COMMAND(SmokeCutDrawCapsuleCommand)(
        [RenderTargetResource, CenterUVCopy, DirectionUVCopy, HalfLengthCopy, RadiusCopy, EdgeSoftnessCopy, StrengthCopy](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            FRDGTextureRef OutputTexture = RenderTargetResource->GetRenderTargetTexture(GraphBuilder);
            if (!OutputTexture)
            {
                GraphBuilder.Execute();
                return;
            }

            TShaderMapRef<FSmokeCutDrawCapsuleCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            FSmokeCutDrawCapsuleCS::FParameters* PassParameters =
                GraphBuilder.AllocParameters<FSmokeCutDrawCapsuleCS::FParameters>();

            const FIntPoint TextureSize = OutputTexture->Desc.Extent;

            PassParameters->CenterUV = FVector2f((float)CenterUVCopy.X, (float)CenterUVCopy.Y);
            PassParameters->DirectionUV = FVector2f((float)DirectionUVCopy.X, (float)DirectionUVCopy.Y);
            PassParameters->HalfLength = HalfLengthCopy;
            PassParameters->Radius = RadiusCopy;
            PassParameters->EdgeSoftness = EdgeSoftnessCopy;
            PassParameters->Strength = StrengthCopy;
            PassParameters->TextureSize = TextureSize;
            PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));

            const FIntVector GroupCount(
                FMath::DivideAndRoundUp(TextureSize.X, 8),
                FMath::DivideAndRoundUp(TextureSize.Y, 8),
                1
            );

            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("SmokeCutCS_DrawCapsule"),
                ComputeShader,
                PassParameters,
                GroupCount
            );

            GraphBuilder.Execute();
        }
    );
}

void USmokeCutShaderLibrary::DispatchDiffuseMask(
    UTextureRenderTarget2D* InputRenderTarget,
    UTextureRenderTarget2D* OutputRenderTarget,
    float DeltaTime,
    float DiffuseStrength
)
{
    if (!InputRenderTarget || !OutputRenderTarget)
    {
        return;
    }

    FTextureRenderTargetResource* InputResource = InputRenderTarget->GameThread_GetRenderTargetResource();
    FTextureRenderTargetResource* OutputResource = OutputRenderTarget->GameThread_GetRenderTargetResource();

    if (!InputResource || !OutputResource)
    {
        return;
    }

    const float DeltaTimeCopy = DeltaTime;
    const float DiffuseStrengthCopy = DiffuseStrength;

    ENQUEUE_RENDER_COMMAND(SmokeCutDiffuseMaskCommand)(
        [InputResource, OutputResource, DeltaTimeCopy, DiffuseStrengthCopy](FRHICommandListImmediate& RHICmdList)
        {
            FRDGBuilder GraphBuilder(RHICmdList);

            FRDGTextureRef InputTexture = InputResource->GetRenderTargetTexture(GraphBuilder);
            FRDGTextureRef OutputTexture = OutputResource->GetRenderTargetTexture(GraphBuilder);

            if (!InputTexture || !OutputTexture)
            {
                GraphBuilder.Execute();
                return;
            }

            TShaderMapRef<FSmokeCutDiffuseMaskCS> ComputeShader(GetGlobalShaderMap(GMaxRHIFeatureLevel));

            FSmokeCutDiffuseMaskCS::FParameters* PassParameters =
                GraphBuilder.AllocParameters<FSmokeCutDiffuseMaskCS::FParameters>();

            const FIntPoint TextureSize = OutputTexture->Desc.Extent;

            PassParameters->DeltaTime = DeltaTimeCopy;
            PassParameters->DiffuseStrength = DiffuseStrengthCopy;
            PassParameters->TextureSize = TextureSize;
            PassParameters->InputTexture = InputTexture;
            PassParameters->InputSampler = TStaticSamplerState<SF_Point>::GetRHI();
            PassParameters->OutputTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(OutputTexture));

            const FIntVector GroupCount(
                FMath::DivideAndRoundUp(TextureSize.X, 8),
                FMath::DivideAndRoundUp(TextureSize.Y, 8),
                1
            );

            FComputeShaderUtils::AddPass(
                GraphBuilder,
                RDG_EVENT_NAME("SmokeCutCS_DiffuseMask"),
                ComputeShader,
                PassParameters,
                GroupCount
            );

            GraphBuilder.Execute();
        }
    );
}