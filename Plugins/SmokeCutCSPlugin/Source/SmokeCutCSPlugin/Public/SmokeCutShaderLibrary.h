#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SmokeCutShaderLibrary.generated.h"

class UTextureRenderTarget2D;

UCLASS()
class SMOKECUTCSPLUGIN_API USmokeCutShaderLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SmokeCutCS")
	static void DispatchSimpleFill(UTextureRenderTarget2D* RenderTarget, FLinearColor FillColor);

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS")
	static void DispatchDrawCircle(
		UTextureRenderTarget2D* RenderTarget,
		FVector2D CenterUV,
		float Radius,
		float EdgeSoftness,
		float Strength);

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS")
	static void DispatchRecoverMask(
		UTextureRenderTarget2D* RenderTarget,
		float DeltaTime,
		float RecoveryRate
	);

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS")
	static void DispatchDrawCapsule(
		UTextureRenderTarget2D* RenderTarget,
		FVector2D CenterUV,
		FVector2D DirectionUV,
		float HalfLength,
		float Radius,
		float EdgeSoftness,
		float Strength
	);

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS")
	static void DispatchDiffuseMask(
		UTextureRenderTarget2D* InputRenderTarget,
		UTextureRenderTarget2D* OutputRenderTarget,
		float DeltaTime,
		float DiffuseStrength
	);
};