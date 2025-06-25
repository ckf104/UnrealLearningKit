// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"
#include "Containers/Array.h"
#include "CoreMinimal.h"
#include "AbilityComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/StaticMesh.h"
#include "Logging/LogMacros.h"
#include "Materials/MaterialInterface.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"
#include "IceAbilityComponent.generated.h"

class AStaticMeshActor;
class ASpawnedIce;
DECLARE_LOG_CATEGORY_EXTERN(IceAbility, Log, All);

/**
 * 
 */
UCLASS()
class UNREALLEARNINGKIT_API UIceAbilityComponent : public UAbilityComponent
{
	GENERATED_BODY()

	UIceAbilityComponent();
	
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float MaxCreateDistance;

	void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Ice Ability")
	void EnableAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ice Ability")
	void EnableAbilityEvent();

	UFUNCTION(BlueprintCallable, Category = "Ice Ability")
	void StartAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ice Ability")
	void StartAbilityEvent();

	UFUNCTION(BlueprintCallable, Category = "Ice Ability")
	void CancelAbility();

	UFUNCTION(BlueprintImplementableEvent, Category = "Ice Ability")
	void CancelAbilityEvent();

	UFUNCTION(BlueprintCallable, Category = "Ice Ability")
	void CreateIce(FVector IceLocation, FRotator IceRotation);

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// TimelineComponent 只能在 Actor 中使用
	UPROPERTY()
	FTimeline IceTimeline;

	/* 冰块被 blocking 时透明度变化曲线 */
	UPROPERTY(EditAnywhere, Category = "Ice Spawning")
	TObjectPtr<UCurveFloat> IceBlockingAlphaCurve;

	/* 生成冰块时的 z scale 变化曲线*/
	UPROPERTY(EditAnyWhere, Category = "Ice Spawning")
	TObjectPtr<UCurveFloat> IceZScaleCurve;

	/* 用于 trace 检测是否能生成冰块的 box 大小, 它的值为实际 box 边长的一半 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Spawning")
	FVector BoxTraceExtent;

	/* Spawn 出的冰块的 scale 设置 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Spawning")
	FVector IceScale;

	/* Spawn 时冰块的位置相对 trace hit point 的偏移 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Spawning")
	FVector IceSpawnOffset;

	/* 冰块在水面下方的深度 */
	UPROPERTY(EditAnywhere, Category = "Ice Spawning")
	float IceDepth;

	/* 冰块在被 block 时的 z scale */
	UPROPERTY(EditAnywhere, Category = "Ice Spawning")
	float IceBlockingZScale;	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Spawning")
	int32 MaxIceNumber;

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<AStaticMeshActor> SpawnedIce;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Spawning")
	TObjectPtr<UStaticMesh> IceMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Spawning")
	TObjectPtr<UMaterialInterface> PreviewIceMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Ice Spawning")
	TSubclassOf<ASpawnedIce> IceClass;

	/* 当冰块没有被 block 时的材质 alpha 值*/
	UPROPERTY(EditDefaultsOnly, Category = "Ice Spawning")
	float DefaultAlpha;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	FName TraceTag;


	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	uint8 bDrawTraceBox : 1;

	UPROPERTY(BlueprintReadWrite, Category = "Ice Spawning")
	uint8 bCreateIceRquested : 1;

private:
	/* 这一帧试图 spawn 的冰块是否与环境发生碰撞 */
	uint8 bIceBlockingThisFrame : 1;

	void OnIceBlockingAlphaCurveUpdate(float Alpha);

	void OnIceZScaleCurveUpdate(float ZScale);

	void ClearResource();

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicIceBlockingMaterial;

	UPROPERTY()
	TArray<TObjectPtr<AActor>> IceActors;
};
