


#include "AbilityComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/HitResult.h"
#include "Kismet/KismetSystemLibrary.h"
#include "KismetTraceUtils.h"
#include "Math/Color.h"
#include "Misc/AssertionMacros.h"


// Sets default values for this component's properties
UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	// ...
}

FHitResult UAbilityComponent::LineTraceFromCamera(float maxDistance) const
{
	FHitResult hitResult;
	FCollisionQueryParams collisionParams;
	collisionParams.AddIgnoredActor(GetOwner());
	collisionParams.bReturnPhysicalMaterial = true;


	APlayerCameraManager* PCM = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
	ensure(PCM);

	FVector start = PCM->GetCameraLocation();
	auto CameraForwardVec = PCM->GetCameraRotation().Vector();
	FVector end = start + CameraForwardVec * maxDistance;

	auto bHit = GetWorld()->LineTraceSingleByChannel(hitResult, start, end, ECC_Visibility, collisionParams);

	if (bDrawTraceLine)
	{
		DrawDebugLineTraceSingle(GetWorld(), start, end, EDrawDebugTrace::ForOneFrame, bHit, hitResult, FColor::Red, FColor::Blue, 0.5f);
	}

	return hitResult;
}
