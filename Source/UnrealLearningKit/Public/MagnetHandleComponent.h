// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Math/MathFwd.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "MagnetHandleComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType, meta=(BlueprintSpawnableComponent))
class UNREALLEARNINGKIT_API UMagnetHandleComponent : public UPhysicsHandleComponent
{
	GENERATED_BODY()
	
public:
	bool GetPhysicsTransform(FVector& Location, FRotator& Rotation) const;
	bool GetKinematicTransform(FVector& Location, FRotator& Rotation) const;
	
};
