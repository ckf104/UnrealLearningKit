// Fill out your copyright notice in the Description page of Project Settings.

#include "RPGBlueprintFunctionLibrary.h"
#include "GenericTeamAgentInterface.h"

HitDirection URPGBlueprintFunctionLibrary::GetHitDirection(const FVector& HitLocation, const AActor* HitActor)
{
	FVector ActorLocation = HitActor->GetActorLocation();
	auto HitDir = (HitLocation - ActorLocation);
	auto HitAngle = HitActor->GetActorRotation().UnrotateVector(HitDir).HeadingAngle();

	auto HitDegree = FMath::RadiansToDegrees(HitAngle);

	if (HitDegree >= -45 && HitDegree < 45)
	{
		return HitDirection::Front;
	}
	else if (HitDegree >= 45 && HitDegree < 135)
	{
		return HitDirection::Right;
	}
	else if (HitDegree >= 135 || HitDegree < -135)
	{
		return HitDirection::Back;
	}
	else
	{
		return HitDirection::Left;
	}
}