// Fill out your copyright notice in the Description page of Project Settings.

#include "LighSphere.h"
#include "Components/StaticMeshComponent.h"
#include "Delegates/Delegate.h"
#include "GameFramework/Actor.h"
#include "Misc/AssertionMacros.h"

ALighSphere::ALighSphere()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
	MoveSpeed = 100.0f;
	bShouldDestory = false;
}

void ALighSphere::BeginPlay()
{
	UE_LOG(LogTemp, Log, TEXT("Light Sphere Begin Play!"));
	Super::BeginPlay();
	GetStaticMeshComponent()->OnComponentBeginOverlap.AddUniqueDynamic(this, &ALighSphere::OverlapCallback);
}

void ALighSphere::OverlapCallback(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ensure(OverlappedComponent == RootComponent);
	ensure(OtherActor != nullptr && OtherComp != nullptr);

	UE_LOG(LogTemp, Log, TEXT("Overlapped with %s"), *OtherActor->GetName());
	if (OtherActor == TargetActor)
	{
		bShouldDestory = true;
		OnLightSphereDestory.Broadcast(OtherComp);
	}
}

void ALighSphere::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// UE_LOG(LogTemp, Log, TEXT("Light Sphere Ticked!"));
	if (MovedDistance > MaxMoveDistance)
	{
		bShouldDestory = true;
		OnLightSphereDestory.Broadcast(nullptr);
	}
	else
	{
		auto Loc = GetActorLocation();
		auto NewLoc = Loc + MoveDir * MoveSpeed * DeltaTime;
		MovedDistance += MoveSpeed * DeltaTime;
		SetActorLocation(NewLoc, true);
	}
	if (bShouldDestory)
	{
		Destroy();
	}
}
