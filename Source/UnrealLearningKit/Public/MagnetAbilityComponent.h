// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityComponent.h"
#include "Math/MathFwd.h"
#include "MagnetAbilityComponent.generated.h"

class AArrowActorBase;
class AMagnetSpline;
class UMagnetHandleComponent;
class ALighSphere;
DECLARE_LOG_CATEGORY_EXTERN(MagnetAbility, Log, All);

/**
 * 
 */
UCLASS()
class UNREALLEARNINGKIT_API UMagnetAbilityComponent : public UAbilityComponent
{
	GENERATED_BODY()	
public:
	UMagnetAbilityComponent();

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Magnet Ability")
	void EnableAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Magnet Ability")
	void EnableAbilityEvent();

	UFUNCTION(BlueprintCallable, Category = "Magnet Ability")
	void StartAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Magnet Ability")
	void StartAbilityEvent();

	UFUNCTION(BlueprintCallable, Category = "Magnet Ability")
	void CancelAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Magnet Ability")
	void CancelAbilityEvent();

	UFUNCTION(BlueprintImplementableEvent, Category = "Magnet Ability", meta = (DisplayName = "Grab Success"))
	void GrabSuccuessEvent();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Magnet Ability")
	AActor* GetGrabbedActor() const;

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/* Magnet Spline 的类型 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TSubclassOf<AActor> MagnetSplineClass;

	/* 光球的类型 */
	UPROPERTY(EditAnywhere, Category = "Magnet Spline")
	TSubclassOf<AActor> SphereClass;

	/* 圆盘的类型 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	TSubclassOf<AActor> DiskActorClass;

	/* 箭头 Actor 的类型 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	TSubclassOf<AActor> ArrowActorClass;

	/* 箭头 Actor 的长度 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float ArrowLength;

	/* 箭头 Actor 横向的长度 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float ArrowXLength;

	/* 箭头 Actor 的最大长度 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float ArrowMaxLength;

	/* 箭头 Actor 下方距离 hit point 的距离 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float ArrowDistFromHitPoint;

	/* 圆盘 Actor 下方距离 hit point 的距离 */
	UPROPERTY(EditAnywhere, Category = "Magnet")
	float DiskDistFromHitPoint;

	/* 最远能抓取到的物体的距离 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Magnet")
	float MaxTraceDistance;

	/* 抓取的物体距角色的最大距离 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Magnet")
	float MaxGrabDistance;

	/* 抓取的物体距角色的最小距离 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Magnet")
	float MinGrabDistance;

	/* 抓取的物体距离地面的最大高度 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Magnet")
	float MaxGrabHeight;

	/* 玩家操纵物体移动时的物体角度变化速度 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Magnet")
	float GrabAngleMoveScale;

	/* 玩家操纵物体移动时的物体高度变化速度 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Magnet")
	float GrabHeightMoveScale;

	/* 玩家操纵物体移动时的物体距玩家位置的变化速度 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Magnet")
	float GrabDistScale;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Magnet")
	float LightSphereInitialDistance;

	UPROPERTY(BlueprintReadWrite, Category = "Magnet")
	FVector InputVector;

	UPROPERTY(BlueprintReadWrite, Category = "Magnet")
	uint8 bGrabObjRquested : 1;

	UPROPERTY(BlueprintReadWrite, Category = "Magnet")
	uint8 bGrabbing : 1;

	UPROPERTY(EditAnywhere, Category = "Magnet")
	uint8 bConstrainRotation : 1;

	UPROPERTY(EditAnywhere, Category = "Debug")
	uint8 bDebugArrowTrace : 1;

	UPROPERTY(EditAnywhere, Category = "Debug")
	uint8 bDebugDrawHitPos : 1;

private:
	UMagnetHandleComponent* GetPhysicsHandle() const;	

	/* 被抓取物体与角色的相对位置 */
	FVector RelativeGrabLocation;

	UPROPERTY()
	TObjectPtr<AArrowActorBase> ArrowActor;

	UPROPERTY()
	TObjectPtr<AActor> DiskActor;

	UPROPERTY()
	TObjectPtr<ALighSphere> SphereActor;

	UPROPERTY()
	TObjectPtr<AMagnetSpline> SplineActor;

	uint8 bTryGrabbing : 1;

	void OnSphereGrabSuccess(UPrimitiveComponent* GrabbedComponent);
};
