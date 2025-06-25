// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/PrimitiveComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "Containers/EnumAsByte.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/MathFwd.h"
#include "UObject/ObjectPtr.h"
#include "MagnetSpline.generated.h"

class UMagnetAbilityComponent;
class UPrimitiveComponent;

USTRUCT()
struct FSplineKeyFrame
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Spline Keyframe")
	float Time;

	UPROPERTY(EditAnywhere, Category = "Spline Keyframe")
	TArray<FVector> Position;

	UPROPERTY(EditAnywhere, Category = "Spline Keyframe")
	TArray<FVector> Tangent;
};

UCLASS()
class UNREALLEARNINGKIT_API AMagnetSpline : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMagnetSpline();

	bool ShouldTickIfViewportsOnly() const override
	{
		return true;
	}

	void SetStartAttachedComp(UPrimitiveComponent* Comp)
	{
		StartAttachedComp = Comp;
	}
	void SetEndAttachedComp(UPrimitiveComponent* Comp)
	{
		EndAttachedComp = Comp;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Magnet Spline")
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TObjectPtr<UStaticMesh> StaticMesh;

	/* 控制每段 spline mesh 的材质 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TArray<TObjectPtr<UMaterialInterface>> Material;

	UPROPERTY(EditAnywhere, Category = "Keyframe Spline")
	TArray<FSplineKeyFrame> KeyFrames;

	/* 是否要使用 keyframe 驱动 spline */
	UPROPERTY(EditAnywhere, Category = "Keyframe Spline")
	uint8 bUseKeyFrames : 1;

	/* 是否使用 cubic hermite spline */
	UPROPERTY(EditAnywhere, Category = "Keyframe Spline")
	uint8 bUseCubicInterp : 1;

	/* cubic hermite spline 中初始点的 tangent 值 */
	UPROPERTY(EditAnywhere, Category = "Keyframe Spline")
	float CubicStartTangent;

	/* cubic hermite spline 中末端点的 tangent 值 */
	UPROPERTY(EditAnywhere, Category = "Keyframe Spline")
	float CubicEndTangent;

	/* 基础的 spline 长度 */
	UPROPERTY(EditAnywhere, Category = "Keyframe Spline")
	float AnimSplineBaseLength;

	/* 控制每个 mesh 在 spline 上的位置 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TArray<int32> InputKeys;

	/* 控制 mesh 顶点的振幅 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TArray<float> Amplitudes;

	/* 控制 mesh 顶点的频率 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TArray<float> Frequencies;

	/* 控制 mesh 顶点的相位 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TArray<float> Phases;

	/* 控制 mesh 顶点的插值速度 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TArray<int32> InterpSpeed;

	/* 控制每段 spline mesh 的长度 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TArray<float> LengthRatios;

	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;

	UPROPERTY(EditAnywhere, Category = "Magnet Spline", meta = (DisplayName = "Fat Scale"))
	FVector2D Fat;

	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	float StaticMeshLength;

	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	int32 DesiredMeshNumber;

	/* 玩家试图移动的方向, X 表示朝着 target 的水平方向移动，Y 表示朝着 target 的水平方向旋转， Z 表示垂直方向的移动 */
	UPROPERTY(BlueprintReadWrite, Category = "Magnet Spline")
	FVector ControlMoveDir;

	UFUNCTION(BlueprintImplementableEvent, Category = "Magnet Spline")
	FVector ConstructScale(float FatScale, float LengthScale);

	UFUNCTION(BlueprintCallable, Category = "Magnet Spline")
	void SetupSplineMeshes();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintNativeEvent, Category = "Magnet Spline")
	double GetSwingStrengthForSplinePoint(int32 SplinePointIndex, double TotalLength, FVector ControlDir);
	double GetSwingStrengthForSplinePoint_Implementation(int32 SplinePointIndex, double TotalLength, FVector ControlDir);

	UFUNCTION(BlueprintNativeEvent, Category = "Magnet Spline")
	double GetXMoveStrengthForSplinePoint(int32 SplinePointIndex, double TotalLength, FVector ControlDir);
	double GetXMoveStrengthForSplinePoint_Implementation(int32 SplinePointIndex, double TotalLength, FVector ControlDir);

	UFUNCTION(BlueprintNativeEvent, Category = "Magnet Spline")
	double GetZMoveStrengthForSplinePoint(int32 SplinePointIndex, double TotalLength, FVector ControlDir);
	double GetZMoveStrengthForSplinePoint_Implementation(int32 SplinePointIndex, double TotalLength, FVector ControlDir);

	UFUNCTION(BlueprintNativeEvent, Category = "Magnet Spline")
	double GetTangentStrengthForSplinePoint(int32 SplinePointIndex, double TotalLength, FVector ControlDir);
	double GetTangentStrengthForSplinePoint_Implementation(int32 SplinePointIndex, double TotalLength, FVector ControlDir);

	/* spline component 的起始点和末端点相对于 Comp 的偏移，如果 Comp 没有启用 simulating physics */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	FVector StartPosOffset;
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	FVector EndPosOffset;

private:	
	friend class UMagnetAbilityComponent;
	float TotalTime;

	/* spline component 的起始点和末端点位于 Comp 的质心 */
	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> EndAttachedComp;

	UPROPERTY()
	TObjectPtr<UPrimitiveComponent> StartAttachedComp;

	void SwingPositionUpdate(float DeltaTime);

	void SplineTangentUpdate();

	void KeyframeSplineUpdate();

	void SetInitialPosition();

	void DumpSplinePosition();

	FVector GetStartAttachPos();
	FVector GetEndAttachPos();
	FVector Interp(const FVector& Start, const FVector& End, float Alpha);

	int32 GetKeyframeIndexAndBlendAlpha(float& Alpha);


	bool KeyframeCheck();
};
