// Fill out your copyright notice in the Description page of Project Settings.

#include "IceAbilityComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LocalPlayerAbilitySubsystem.h"
#include "SpawnedIce.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Curves/CurveFloat.h"
#include "Engine/EngineTypes.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "KismetTraceUtils.h"
#include "Logging/LogMacros.h"
#include "Math/MathFwd.h"
#include "Misc/AssertionMacros.h"
#include "WaterActor.h"

DEFINE_LOG_CATEGORY(IceAbility)

UIceAbilityComponent::UIceAbilityComponent()
{
	MaxCreateDistance = 1000.0f;
	DefaultAlpha = 0.75f;
	MaxIceNumber = 3;

	TraceTag = TEXT("IceAbilityTrace");
}

void UIceAbilityComponent::BeginPlay()
{
	Super::BeginPlay();
	IceTimeline.AddInterpFloat(IceBlockingAlphaCurve, FOnTimelineFloatStatic::CreateUObject(this, &UIceAbilityComponent::OnIceBlockingAlphaCurveUpdate));
	IceTimeline.AddInterpFloat(IceZScaleCurve, FOnTimelineFloatStatic::CreateUObject(this, &UIceAbilityComponent::OnIceZScaleCurveUpdate));
}

void UIceAbilityComponent::OnIceBlockingAlphaCurveUpdate(float value)
{
	if (SpawnedIce && bIceBlockingThisFrame)
	{
		auto mesh = SpawnedIce->GetStaticMeshComponent();
		// UE_LOG(IceAbility, Log, TEXT("Alpha: %f"), value);
		mesh->SetScalarParameterValueOnMaterials(TEXT("Alpha"), value);
	}
}

void UIceAbilityComponent::OnIceZScaleCurveUpdate(float ZScale)
{
	if (SpawnedIce && !bIceBlockingThisFrame)
	{
		auto mesh = SpawnedIce->GetStaticMeshComponent();
		FVector NewScale = { IceScale.X, IceScale.Y, IceScale.Z * ZScale };
		mesh->SetWorldScale3D(NewScale);
	}
}

void UIceAbilityComponent::EnableAbility()
{
	EnableAbilityEvent();
}

void UIceAbilityComponent::StartAbility()
{
	Activate();
	StartAbilityEvent();
	if (auto* AbilitySubsystem = ULocalPlayerAbilitySubsystem::Get(GetWorld()))
	{
		AbilitySubsystem->AbilityStatusChangedEvent.Broadcast(EAbilityType::Ice, EAbilityStatus::Inactive, EAbilityStatus::Start);
	}
}

void UIceAbilityComponent::CancelAbility()
{
	Deactivate();
	ClearResource();
	CancelAbilityEvent();
	if (auto* AbilitySubsystem = ULocalPlayerAbilitySubsystem::Get(GetWorld()))
	{
		AbilitySubsystem->AbilityStatusChangedEvent.Broadcast(EAbilityType::Ice, EAbilityStatus::Start, EAbilityStatus::Inactive);
	}
}

void UIceAbilityComponent::ClearResource()
{
	if (SpawnedIce)
	{
		SpawnedIce->Destroy();
		SpawnedIce = nullptr;
	}
	DynamicIceBlockingMaterial = nullptr;
	bIceBlockingThisFrame = false;
	bCreateIceRquested = false;
	IceTimeline.Stop();
}

void UIceAbilityComponent::CreateIce(FVector IceLocation, FRotator IceRotation)
{
	//	FCollisionQueryParams QueryParam;
	//	FCollisionObjectQueryParams ObjectQueryParams(ECollisionChannel::ECC_PhysicsBody);
	//	ObjectQueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);

	//	TArray<FOverlapResult> OverlapResults;
	//	auto InitialZScale = IceZScaleCurve->GetFloatValue(0.0f);
	//	auto BoxInitialExtent = FVector(BoxTraceExtent.X, BoxTraceExtent.Y, BoxTraceExtent.Z * InitialZScale);
	//	GetWorld()->OverlapMultiByObjectType(OverlapResults, IceLocation, IceRotation.Quaternion(), ObjectQueryParams, FCollisionShape::MakeBox(BoxInitialExtent), QueryParam);

	auto IceQuat = IceRotation.Quaternion();
	auto ReverseZ = FRotator{ 180.0f, 0.0f, 0.0f }.Quaternion();
	auto FinalIceRotation = (ReverseZ * IceQuat).Rotator();
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.bDeferConstruction = true;
	auto* NewIce = Cast<ASpawnedIce>(GetWorld()->SpawnActor(IceClass, &IceLocation, &FinalIceRotation, SpawnParams));

	if (IceActors.Num() >= MaxIceNumber)
	{
		AActor* OldActor = IceActors[0];
		OldActor->Destroy();
		// TODO: 播放破碎效果
		IceActors.RemoveAt(0);
	}
	IceActors.Add(NewIce);

	NewIce->MoveDir = IceRotation.RotateVector(FVector::UpVector);
	NewIce->TargetDist = BoxTraceExtent.Z * IceZScaleCurve->GetFloatValue(0.0f);
	NewIce->FinishSpawning(FTransform(FinalIceRotation, IceLocation));

	CancelAbility();
}

void UIceAbilityComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	auto HitResult = LineTraceFromCamera(MaxCreateDistance);
	if (!HitResult.bBlockingHit || !Cast<AWaterActor>(HitResult.GetActor()))
	{
		ClearResource();
		goto Over;
	}
	{
		// Box trace 确认是否能生成冰块
		FVector zDir = HitResult.ImpactNormal;
		FVector xDir = { 1.0, 0.0, 0.0 };

		FMatrix rotationMatrix = FRotationMatrix::MakeFromXZ(xDir, zDir);
		auto BoxQuat = rotationMatrix.ToQuat();
		FVector TracePos = HitResult.ImpactPoint + HitResult.ImpactNormal * (BoxTraceExtent.Z - IceDepth);

		FCollisionQueryParams QueryParam;
		// 忽略 character 和 water, 我们会 disable collision of preview ice
		QueryParam.AddIgnoredActor(GetOwner());
		QueryParam.AddIgnoredActor(HitResult.GetActor());
		QueryParam.TraceTag = TraceTag;

		// 检测所有的 overlap
		FCollisionObjectQueryParams ObjectQueryParams(FCollisionObjectQueryParams::AllObjects);
		ObjectQueryParams.RemoveObjectTypesToQuery(ECollisionChannel::ECC_PhysicsBody);

		auto bIceBlockingLastFrame = bIceBlockingThisFrame;
		// bIceBlockingThisFrame = GetWorld()->SweepTestByChannel(TracePos, TracePos, BoxQuat, ECollisionChannel::ECC_Visibility, FCollisionShape::MakeBox(BoxTraceExtent), QueryParam);
		bIceBlockingThisFrame = GetWorld()->SweepTestByObjectType(TracePos, TracePos, BoxQuat, ObjectQueryParams, FCollisionShape::MakeBox(BoxTraceExtent), QueryParam);

		if (bDrawTraceBox)
		{
			DrawDebugSweptBox(GetWorld(), TracePos, TracePos, rotationMatrix.Rotator(), BoxTraceExtent, FColor::Green);
		}
		auto NewIceLocation = HitResult.Location - HitResult.ImpactNormal * IceDepth + IceSpawnOffset;

		if (!SpawnedIce)
		{
			ensure(!DynamicIceBlockingMaterial);
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

			SpawnedIce = GetWorld()->SpawnActor<AStaticMeshActor>(NewIceLocation, rotationMatrix.Rotator(), SpawnParams);
			SpawnedIce->SetMobility(EComponentMobility::Movable);
			// 设置初始的 scale，z 方向的 scale 会在 timeline 中更新
			SpawnedIce->SetActorScale3D(IceScale);
			SpawnedIce->SetActorEnableCollision(false);
			auto* SMC = SpawnedIce->GetStaticMeshComponent();
			SMC->SetStaticMesh(IceMesh);

			DynamicIceBlockingMaterial = SMC->CreateDynamicMaterialInstance(0, PreviewIceMaterial);
			ensure(DynamicIceBlockingMaterial);
		}
		else
		{
			SpawnedIce->SetActorLocation(NewIceLocation);
		}

		// ice blocking 时修改 z scale 为 `IceBlockingZScale`
		if (bIceBlockingThisFrame && !bIceBlockingLastFrame)
		{
			SpawnedIce->SetActorScale3D(FVector{ IceScale.X, IceScale.Y, IceBlockingZScale });
		}

		if (!bIceBlockingThisFrame)
		{
			// UE_LOG(IceAbility, Log, TEXT("Set alpha to %f"), DefaultAlpha);
			SpawnedIce->GetStaticMeshComponent()->SetScalarParameterValueOnMaterials(TEXT("Alpha"), DefaultAlpha);
		}

		if (bCreateIceRquested && !bIceBlockingThisFrame)
		{
			UE_LOG(IceAbility, Log, TEXT("Create ice"));
			CreateIce(NewIceLocation, rotationMatrix.Rotator());
		}
		else
		{
			bCreateIceRquested = false;
			// 从其它状态切换到 line trace hit water 时，或者从 ice blocking 状态切换时，从头播放 timeline
			if (!IceTimeline.IsPlaying() || bIceBlockingLastFrame != bIceBlockingThisFrame)
			{
				IceTimeline.PlayFromStart();
				IceTimeline.SetLooping(true);
			}
			else
			{
				IceTimeline.TickTimeline(DeltaTime);
			}
		}
	}
Over:
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
