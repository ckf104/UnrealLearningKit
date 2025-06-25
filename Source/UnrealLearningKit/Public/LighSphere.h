// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "UObject/ObjectPtr.h"
#include "LighSphere.generated.h"

class UPrimitiveComponent;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLightSphereDestoryEvent, UPrimitiveComponent*);

/**
 * 
 */
UCLASS()
class UNREALLEARNINGKIT_API ALighSphere : public AStaticMeshActor
{
	GENERATED_BODY()
	
	
public:
	ALighSphere();

	void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Light Sphere")	
	float MoveSpeed;
	
	FVector MoveDir;

	float MaxMoveDistance;

	FOnLightSphereDestoryEvent OnLightSphereDestory;

	UPROPERTY()
	TObjectPtr<AActor> TargetActor;

	void Tick(float DeltaTime) override;

	UFUNCTION()
	void OverlapCallback(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

private:
	float MovedDistance;

	int32 bShouldDestory: 1;
};
