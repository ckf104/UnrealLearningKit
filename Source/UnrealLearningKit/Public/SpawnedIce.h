// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/StaticMeshActor.h"
#include "SpawnedIce.generated.h"

class UPrimitiveComponent;

/**
 * 
 */
UCLASS()
class UNREALLEARNINGKIT_API ASpawnedIce : public AStaticMeshActor
{
	GENERATED_BODY()

protected:
	void Tick(float DeltaTime) override;
	
public:
	ASpawnedIce();

	UPROPERTY(BlueprintReadWrite, Category = "Ice Spawning")	
	FVector MoveDir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Spawning")
	float PushStep;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Spawning")
	float TargetDist;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Ice Spawning")
	int32 MaxPushNumber;

	UFUNCTION(BlueprintCallable, Category = "Ice Spawning")
	void SweepDistance();

private:
	bool CanPushComp(UPrimitiveComponent* Comp);

	bool bMoveFail;
	float MovedDist;
};
