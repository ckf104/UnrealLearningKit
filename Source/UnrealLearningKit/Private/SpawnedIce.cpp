// Fill out your copyright notice in the Description page of Project Settings.

#include "SpawnedIce.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Containers/Set.h"
#include "Engine/HitResult.h"
#include "GameFramework/Character.h"
#include "Math/UnrealMathUtility.h"
#include "Templates/Casts.h"

ASpawnedIce::ASpawnedIce()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	MoveDir = FVector{ 0.0f, 0.0f, 1.0f };
	PushStep = 10.0f;
	MaxPushNumber = 10;
}

bool ASpawnedIce::CanPushComp(UPrimitiveComponent* Comp)
{
	if (!Comp)
	{
		return false;
	}
	if (Comp->IsSimulatingPhysics())
	{
		return true;
	}
	if (auto* Char = Cast<ACharacter>(Comp->GetOwner()))
	{
		return true;
	}
	return false;
}

void ASpawnedIce::SweepDistance()
{
	if (bMoveFail || TargetDist <= MovedDist + UE_KINDA_SMALL_NUMBER)
	{
		return;
	}
	auto* Mesh = GetStaticMeshComponent();
	int32 PushCount = 0;
	while (PushCount < MaxPushNumber && TargetDist > MovedDist + UE_KINDA_SMALL_NUMBER)
	{
		auto DiffDist = TargetDist - MovedDist;
		auto SweepDist = DiffDist * MoveDir;
		FHitResult HitResult;
		Mesh->MoveComponent(SweepDist, Mesh->GetComponentQuat(), true, &HitResult);
		MovedDist += HitResult.Time * DiffDist;
		UE_LOG(LogTemp, Log, TEXT("MovedDist: %f"), MovedDist);
		if (HitResult.bBlockingHit)
		{
			if (CanPushComp(HitResult.GetComponent()))
			{
				HitResult.Component->AddWorldOffset(FMath::Max(PushStep, DiffDist) * MoveDir, false, nullptr, ETeleportType::TeleportPhysics);
				PushCount++;
			}
			else
			{
        UE_LOG(LogTemp, Log, TEXT("MoveFailed"));
				bMoveFail = true;
				break;
			}
		}
	}
}

void ASpawnedIce::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SweepDistance();
}
