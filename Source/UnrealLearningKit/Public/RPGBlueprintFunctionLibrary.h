// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RPGCharacter.h"
#include "RPGBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class UNREALLEARNINGKIT_API URPGBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Action")
	static HitDirection GetHitDirection(const FVector& HitLocation, const AActor* HitActor);
};
