// Fill out your copyright notice in the Description page of Project Settings.

#include "MagnetAbilityComponent.h"
#include "LocalPlayerAbilitySubsystem.h"
#include "MagnetSpline.h"
#include "LighSphere.h"
#include "ArrowActorBase.h"
#include "Engine/HitResult.h"
#include "Engine/World.h"
#include "Kismet/KismetSystemLibrary.h"
#include "KismetTraceUtils.h"
#include "MagnetHandleComponent.h"
#include "Math/MathFwd.h"
#include "Math/UnrealMathUtility.h"
#include "PhysicsEngine/PhysicsHandleComponent.h"
#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"
#include "Misc/AssertionMacros.h"
#include "PaperSpriteComponent.h"

DEFINE_LOG_CATEGORY(MagnetAbility)

UMagnetAbilityComponent::UMagnetAbilityComponent()
{
	MaxTraceDistance = 1000.0f;
	MaxGrabDistance = 1000.0f;
	MaxGrabHeight = 1000.0f;

	ArrowMaxLength = 2000.0f;
	ArrowLength = 400.0f;
	ArrowXLength = 400 * 0.0625f;
	ArrowDistFromHitPoint = 2.0f;
	DiskDistFromHitPoint = 1.0f;

	GrabAngleMoveScale = 0.05f;
	GrabHeightMoveScale = 1.0f;
	GrabDistScale = 1.0f;

	MinGrabDistance = 50.0f;
	LightSphereInitialDistance = MinGrabDistance;
}

void UMagnetAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMagnetAbilityComponent::EnableAbility()
{
	EnableAbilityEvent();
}
void UMagnetAbilityComponent::StartAbility()
{
	Activate();
	StartAbilityEvent();
	if (auto* AbilitySubsystem = ULocalPlayerAbilitySubsystem::Get(GetWorld()))
	{
		AbilitySubsystem->AbilityStatusChangedEvent.Broadcast(EAbilityType::Magnet, EAbilityStatus::Inactive, EAbilityStatus::Start);
	}
}

void UMagnetAbilityComponent::CancelAbility()
{
	Deactivate();
	CancelAbilityEvent();
	if (auto* AbilitySubsystem = ULocalPlayerAbilitySubsystem::Get(GetWorld()))
	{
		AbilitySubsystem->AbilityStatusChangedEvent.Broadcast(EAbilityType::Magnet, EAbilityStatus::Start, EAbilityStatus::Inactive);
	}

	if (bGrabbing)
	{
		auto* PhysicsHandle = GetPhysicsHandle();
		PhysicsHandle->ReleaseComponent();
		bGrabbing = false;
	}
	if (ArrowActor)
	{
		ArrowActor->Destroy();
		ArrowActor = nullptr;
	}
	if (DiskActor)
	{
		DiskActor->Destroy();
		DiskActor = nullptr;
	}
	if (SphereActor)
	{
		// 我们可能希望在 sphere 到达终点前就 cancel 磁铁
		SphereActor->Destroy();
		SphereActor = nullptr;
	}
	if (SplineActor)
	{
		SplineActor->Destroy();
		SplineActor = nullptr;
	}
	bGrabObjRquested = false;
	bTryGrabbing = false;
	InputVector = FVector::ZeroVector;
}

UMagnetHandleComponent* UMagnetAbilityComponent::GetPhysicsHandle() const
{
	auto* Owner = GetOwner();
	UMagnetHandleComponent* PhysicsHandle = nullptr;
	if (Owner)
	{
		PhysicsHandle = Cast<UMagnetHandleComponent>(Owner->GetComponentByClass(UPhysicsHandleComponent::StaticClass()));
	}
	ensure(PhysicsHandle);
	return PhysicsHandle;
}

AActor* UMagnetAbilityComponent::GetGrabbedActor() const
{
	auto* PhysicsHandle = GetPhysicsHandle();
	if (PhysicsHandle)
	{
		return PhysicsHandle->GetGrabbedComponent()->GetOwner();
	}
	return nullptr;
}

void UMagnetAbilityComponent::OnSphereGrabSuccess(UPrimitiveComponent* GrabbedComponent)
{
	if (GrabbedComponent)
	{
		ensure(SphereActor);
		ensure(SplineActor);
		ensure(!ArrowActor && !DiskActor);
		ensure(GrabbedComponent->IsSimulatingPhysics());

		SphereActor = nullptr;
		bGrabbing = true;

		SplineActor->SetEndAttachedComp(GrabbedComponent);

		auto MassCenter = GrabbedComponent->GetCenterOfMass();
		auto* PhysicsHandle = GetPhysicsHandle();
		if (bConstrainRotation)
		{
			PhysicsHandle->GrabComponentAtLocationWithRotation(GrabbedComponent, NAME_None, MassCenter, FRotator());
		}
		else
		{
			PhysicsHandle->GrabComponentAtLocation(GrabbedComponent, NAME_None, MassCenter);
		}
		RelativeGrabLocation = MassCenter - GetOwner()->GetActorLocation();

		bGrabbing = true;
		GrabSuccuessEvent();
		if (auto* AbilitySubsystem = ULocalPlayerAbilitySubsystem::Get(GetWorld()))
		{
			AbilitySubsystem->AbilityStatusChangedEvent.Broadcast(EAbilityType::Magnet, EAbilityStatus::Start, EAbilityStatus::Active);
		}

		UE_LOG(MagnetAbility, Log, TEXT("Grab %s at %s"), *GrabbedComponent->GetOwner()->GetName(), *MassCenter.ToString());

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		ArrowActor = Cast<AArrowActorBase>(GetWorld()->SpawnActor(ArrowActorClass, nullptr, nullptr, SpawnParams));
		DiskActor = GetWorld()->SpawnActor(DiskActorClass, nullptr, nullptr, SpawnParams);

		auto* Comp = DiskActor->GetComponentByClass<UPaperSpriteComponent>();
		ensure(Comp->Mobility == EComponentMobility::Movable);
	}
	else
	{
		CancelAbility();
	}
}

void UMagnetAbilityComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	if (bGrabObjRquested && !bGrabbing && !bTryGrabbing)
	{
		auto HitResult = LineTraceFromCamera(MaxTraceDistance);
		auto* HitActor = HitResult.GetActor();
		auto* HitComponent = HitResult.GetComponent();
		ensure(SphereActor == nullptr && SplineActor == nullptr);

		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.bDeferConstruction = true;
		SplineActor = Cast<AMagnetSpline>(GetWorld()->SpawnActor(MagnetSplineClass, nullptr, nullptr, SpawnParams));

		FVector MoveDir;
		auto bTraceSuccess = HitResult.bBlockingHit && HitResult.GetActor()->ActorHasTag(TEXT("Magnet")) && HitComponent->IsSimulatingPhysics();
		if (bTraceSuccess)
		{
			MoveDir = (HitComponent->GetCenterOfMass() - GetOwner()->GetActorLocation()).GetSafeNormal();
		}
		else
		{
			APlayerCameraManager* PCM = GetWorld()->GetFirstPlayerController()->PlayerCameraManager;
			FVector CameraStart = PCM->GetCameraLocation();
			auto CameraForwardVec = PCM->GetCameraRotation().Vector();
			FVector TraceEnd = CameraStart + CameraForwardVec * MaxTraceDistance;
			MoveDir = (TraceEnd - GetOwner()->GetActorLocation()).GetSafeNormal();
		}
		auto SphereLoc = GetOwner()->GetActorLocation() + MoveDir * LightSphereInitialDistance;
		SpawnParams.bDeferConstruction = false;
		SphereActor = Cast<ALighSphere>(GetWorld()->SpawnActor(SphereClass, &SphereLoc, nullptr, SpawnParams));

		SphereActor->MoveDir = MoveDir;
		SphereActor->MaxMoveDistance = MaxTraceDistance;
		SphereActor->OnLightSphereDestory.AddUObject(this, &UMagnetAbilityComponent::OnSphereGrabSuccess);
		if (bTraceSuccess)
		{
			SphereActor->TargetActor = HitActor;
		}

		SplineActor->SetStartAttachedComp(Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent()));
		SplineActor->SetEndAttachedComp(Cast<UPrimitiveComponent>(SphereActor->GetRootComponent()));
		// 在设置好了 attach component 后再调用 begin play
		SplineActor->FinishSpawning(FTransform());

		bTryGrabbing = true;
	}

	if (bGrabbing)
	{
		auto DistInput = FMath::Clamp(InputVector.X, -1.f, 1.f) * GrabDistScale * DeltaTime;
		auto HeightInput = FMath::Clamp(InputVector.Z, -1.f, 1.f) * GrabHeightMoveScale * DeltaTime;
		auto AngleInput = FMath::Clamp(InputVector.Y, -1.f, 1.f) * GrabAngleMoveScale * DeltaTime;
		auto* PhysicsHandle = GetPhysicsHandle();
		FVector CurrentTargetLocation;
		FRotator CurrentTargetRotation;
		PhysicsHandle->GetTargetLocationAndRotation(CurrentTargetLocation, CurrentTargetRotation);
		auto ActorLocation = GetOwner()->GetActorLocation();
		CurrentTargetLocation = CurrentTargetLocation - ActorLocation;

		// Rotate XY component of CurrentTargetLocation clockwise by AngleInput
		float CosAngle = FMath::Cos(AngleInput);
		float SinAngle = FMath::Sin(AngleInput);
		float NewX = RelativeGrabLocation.X * CosAngle - RelativeGrabLocation.Y * SinAngle;
		float NewY = RelativeGrabLocation.X * SinAngle + RelativeGrabLocation.Y * CosAngle;

		FVector NewTargetLoc = FVector(NewX, NewY, 0.0);
		// Extend length of vector by DistInput
		NewTargetLoc = NewTargetLoc.GetSafeNormal2D() * (FMath::Clamp(NewTargetLoc.Size2D() + DistInput, MinGrabDistance, MaxGrabDistance));
		// Adjust Z component by HeightInput
		NewTargetLoc.Z = FMath::Clamp(RelativeGrabLocation.Z + HeightInput, -MaxGrabDistance, MaxGrabDistance);
		// Set the new target location
		PhysicsHandle->SetTargetLocation(NewTargetLoc + ActorLocation);
		RelativeGrabLocation = NewTargetLoc;

		FVector PhysLoc, PhysTargetLoc;
		FRotator PhysRot, PhysTargetRot;
		PhysicsHandle->GetPhysicsTransform(PhysLoc, PhysRot);
		PhysicsHandle->GetKinematicTransform(PhysTargetLoc, PhysTargetRot);
		// UE_LOG(MagnetAbility, Log, TEXT("PhysLoc: %s, PhysRot, %s, PhysTargetLoc: %s, PhysTargetRot: %s"), *PhysLoc.ToString(), *PhysRot.ToString(), *PhysTargetLoc.ToString(), *PhysTargetRot.ToString());

		// 设置 arrow 的 Y scale
		FHitResult HitResult;
		auto* Priv = GetGrabbedActor()->GetComponentByClass<UPrimitiveComponent>();
		auto StartLoc = Priv->GetCenterOfMass();
		auto EndLoc = StartLoc - FVector(0, 0, ArrowMaxLength);
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(GetGrabbedActor());
		auto bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECollisionChannel::ECC_Visibility, Params);

		if (bDebugArrowTrace)
		{
			DrawDebugLineTraceSingle(GetWorld(), StartLoc, EndLoc, EDrawDebugTrace::ForOneFrame, bHit, HitResult, FColor::Red, FColor::Blue, 0.5f);
		}

		auto HitLoc = HitResult.Location;
		auto ArrowLoc = HitLoc + FVector(0, 0, ArrowDistFromHitPoint);
		auto NewScale = (StartLoc.Z - ArrowLoc.Z) / ArrowLength;
		ArrowActor->SetYScale(NewScale);

		// 设置 arrow 的 rotation 面向摄像机
		auto Rotation = ArrowActor->UpdateRotation();
		// 设置 arrow 的位置
		auto XAxis = Rotation.RotateVector(FVector(1.0, 0.0, 0.0));
		ArrowActor->SetActorLocation(-XAxis * ArrowXLength / 2 + ArrowLoc);

		auto DiskLoc = HitLoc + FVector(0, 0, DiskDistFromHitPoint);
		DiskActor->SetActorLocation(DiskLoc);
		// auto* Mesh = ArrowActor->GetComponentByClass<UPrimitiveComponent>();
		// UE_LOG(MagnetAbility, Log, TEXT("Mesh Location: %s, Mesh Scale: %s"), *Mesh->GetComponentLocation().ToString(), *Mesh->GetComponentScale().ToString());

		if (bDebugDrawHitPos)
		{
			DrawDebugSphere(GetWorld(), HitLoc, 10.0f, 12, FColor::Green, false, -1.0f);
			DrawDebugSphere(GetWorld(), ArrowLoc, 10.0f, 12, FColor::Red, false, -1.0f);
			DrawDebugSphere(GetWorld(), DiskLoc, 10.0f, 12, FColor::Blue, false, -1.0f);
		}
	}

	bGrabObjRquested = false;
	InputVector = FVector::ZeroVector;
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
