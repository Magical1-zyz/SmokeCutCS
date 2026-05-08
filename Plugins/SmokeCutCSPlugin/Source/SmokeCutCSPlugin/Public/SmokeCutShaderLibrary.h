#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "SmokeCutShaderLibrary.generated.h"

class UTextureRenderTarget2D;
class UTextureRenderTargetVolume;

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

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS|Volume3D")
	static void DispatchClearVolume(
		UTextureRenderTargetVolume* RenderTarget,
		FLinearColor ClearColor
	);

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS|Volume3D")
	static void DispatchDrawSphereVolume(
		UTextureRenderTargetVolume* RenderTarget,
		FVector CenterUVW,
		float Radius,
		float EdgeSoftness,
		float Strength
	);

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS|Volume3D")
	static void DispatchRecoverVolume(
		UTextureRenderTargetVolume* RenderTarget,
		float DeltaTime,
		float RecoveryRate
	);

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS|Volume3D")
	static void DispatchDiffuseVolume(
		UTextureRenderTargetVolume* InputRenderTarget,
		UTextureRenderTargetVolume* OutputRenderTarget,
		float DeltaTime,
		float DiffuseStrength
	);

	UFUNCTION(BlueprintCallable, Category="SmokeCutCS|Volume3D")
	static void DispatchDrawCapsuleVolume(
		UTextureRenderTargetVolume* RenderTarget,
		FVector StartUVW,
		FVector EndUVW,
		float Radius,
		float EdgeSoftness,
		float Strength
	);
};