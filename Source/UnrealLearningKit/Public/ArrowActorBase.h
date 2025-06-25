// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Math/MathFwd.h"
#include "ArrowActorBase.generated.h"

UCLASS(Blueprintable, BlueprintType, Abstract)
class UNREALLEARNINGKIT_API AArrowActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArrowActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Arrow")
	void SetYScale(float NewYScale);

	UFUNCTION(BlueprintImplementableEvent, Category = "Arrow")
	FRotator UpdateRotation();
};
