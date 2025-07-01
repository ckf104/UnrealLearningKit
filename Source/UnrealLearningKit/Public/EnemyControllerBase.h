// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyControllerBase.generated.h"

enum class RPGTeam : uint8
{
	None = 0,
	Player = 1,
	Enemy = 2,
	Neutral = 3
};

/**
 *
 */
UCLASS()
class UNREALLEARNINGKIT_API AEnemyControllerBase : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyControllerBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, Category = "Action")
	void UpdateFocus(class AActor* NewFocus, const FAIStimulus& Stimulus);
};
