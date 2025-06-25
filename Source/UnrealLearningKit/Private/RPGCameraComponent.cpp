// Fill out your copyright notice in the Description page of Project Settings.

#include "RPGCameraComponent.h"
#include "CoreGlobals.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LocalPlayerAbilitySubsystem.h"
#include "Logging/LogMacros.h"
#include "Logging/LogVerbosity.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/EnumClassFlags.h"
#include "GameFramework/Actor.h"

URPGCameraComponent::URPGCameraComponent()
		: Super()
{
	// 初始化摄像机偏移
	MagnetSocketOffset = FVector(0.0f, 40.0f, 40.0f); // 默认偏移
	CameraFlags = ECameraFlags::TopBroadView;					// 默认摄像机模式为 TopBroadView

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void URPGCameraComponent::OnCameraModeChanged(EAbilityType, EAbilityStatus PreviousStatus, EAbilityStatus NewStatus)
{
	if (NewStatus == EAbilityStatus::Start)
	{
		SetCameraMode(ECameraMode::Magnet);
	}
	else if (NewStatus == EAbilityStatus::Inactive)
	{
		SetCameraMode(ECameraMode::Normal);
	}
}

void URPGCameraComponent::SetCameraMode(ECameraMode NewCameraMode)
{
	switch (NewCameraMode)
	{
		case ECameraMode::Normal:
			CameraFlags = ECameraFlags::TopBroadView; // Clear MagnetOffset flag
			break;
		case ECameraMode::Magnet:
			CameraFlags = ECameraFlags::TopBroadView | ECameraFlags::MagnetOffset; // Set MagnetOffset flag
			break;
		default:
			break;
	}
}

void URPGCameraComponent::UpdateSpringArmLength(float DeltaTime)
{
	if (!SpringArm)
	{
		return; // 如果没有 Spring Arm 组件，则不进行任何操作
	}

	// 如果摄像机模式为 TopBroadView，则增加 Spring Arm 的长度
	if (EnumHasAnyFlags(CameraFlags, ECameraFlags::TopBroadView))
	{
		float PitchAngle = -FMath::ClampAngle(SpringArm->GetTargetRotation().Pitch, -90.0f, 0.0f);
		float DesiredLength = FMath::Lerp(DefaultSpringArmLength, SpringArmMaxLength, PitchAngle / 90.0f);
		SpringArm->TargetArmLength = FMath::FInterpTo(SpringArm->TargetArmLength, DesiredLength, DeltaTime, SpringArmLengthChangeSpeed);
		// UE_LOG(LogTemp, Log, TEXT("Spring Arm Length: %f, Desired Length: %f, Pitch Angle: %f"), SpringArm->TargetArmLength, DesiredLength, FMath::Fmod(SpringArm->GetTargetRotation().Pitch, 360.0f));
	}
	else
	{
		SpringArm->TargetArmLength = DefaultSpringArmLength; // 恢复默认长度
	}
}

void URPGCameraComponent::UpdateSocketOffset(float DeltaTime)
{
	FVector CurrentOffset = SpringArm ? SpringArm->SocketOffset : FVector::ZeroVector; // 获取当前的 Socket 偏移
	// 如果摄像机模式为 Magnet，则应用偏移
	if (EnumHasAnyFlags(CameraFlags, ECameraFlags::MagnetOffset))
	{
		// 计算当前偏移
		FVector TargetOffset = MagnetSocketOffset;
		if (SpringArm)
		{
			SpringArm->TargetOffset = TargetOffset;
		}

		// 插值当前偏移到目标偏移
		CurrentOffset = FMath::VInterpTo(CurrentOffset, TargetOffset, DeltaTime, MagnetOffsetSpeed);
	}
	else
	{
		// 如果不是 Magnet 模式，则复原偏移
		CurrentOffset = FMath::VInterpTo(CurrentOffset, FVector::ZeroVector, DeltaTime, OffsetRestoreSpeed);
	}
	if (SpringArm)
	{
		SpringArm->SocketOffset = CurrentOffset; // 更新 Spring Arm 的 Socket 偏移
	}
}

void URPGCameraComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	UpdateSocketOffset(DeltaTime);
	if (EnumHasAnyFlags(CameraFlags, ECameraFlags::TopBroadView))
	{
		UpdateSpringArmLength(DeltaTime);
	}
}

void URPGCameraComponent::BeginPlay()
{
	Super::BeginPlay();
	// 初始化摄像机模式为 Normal
	SetCameraMode(ECameraMode::Normal);
	SpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
	if (SpringArm)
	{
		DefaultSpringArmLength = SpringArm->TargetArmLength; // 保存默认的 Spring Arm 长度
																												 // scene component 中 parent 总是先于 child 进行 tick
																												 // SpringArm->AddTickPrerequisiteComponent(this);
	}
	// 获取 LocalPlayerAbilitySubsystem 并注册回调
	if (auto* AbilitySubsystem = ULocalPlayerAbilitySubsystem::Get(GetWorld()))
	{
		AbilitySubsystem->AbilityStatusChangedEvent.AddUObject(this, &URPGCameraComponent::OnCameraModeChanged);
	}
}
