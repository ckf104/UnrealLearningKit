// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "LocalPlayerAbilitySubsystem.generated.h"

/**
 *
 */
UENUM(BlueprintType)
enum class EAbilityType : uint8
{
	Ice UMETA(DisplayName = "Ice"),
	Magnet UMETA(DisplayName = "Magnet"),
	// 可根据需要添加更多能力类型
};

UENUM(BlueprintType)
enum class EAbilityStatus : uint8
{
	Inactive UMETA(DisplayName = "Inactive"),
	Start UMETA(DisplayName = "Start"),
	Active UMETA(DisplayName = "Active"),
};

UCLASS()
class UNREALLEARNINGKIT_API ULocalPlayerAbilitySubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	uint8 IceAbilityActive : 1;

	UPROPERTY(BlueprintReadWrite)
	uint8 MagnetAbilityActive : 1;

	// 静态多播委托，包含三个参数
	DECLARE_MULTICAST_DELEGATE_ThreeParams(FAbilityStatusChangedEvent, EAbilityType /*AbilityType*/, EAbilityStatus /*PreviousStatus*/, EAbilityStatus /*NewStatus*/);

	FAbilityStatusChangedEvent AbilityStatusChangedEvent;

	static ULocalPlayerAbilitySubsystem* Get(UWorld* World, int32 PlayerIndex = 0);
};
