// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "LocalPlayerAbilitySubsystem.h"
#include "RPGCameraComponent.generated.h"

class USpringArmComponent;

enum class ECameraFlags : uint8
{
	TopBroadView = 0x01,  // 摄像机在高处时距离更远
	MagnetOffset = 0x02,  // 相对人物摄像机位置偏移
};

UENUM(BlueprintType)
enum class ECameraMode : uint8
{
	Normal,
	Magnet,
};

ENUM_CLASS_FLAGS(ECameraFlags);

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class UNREALLEARNINGKIT_API URPGCameraComponent : public UCameraComponent
{
	GENERATED_BODY()
	
public:
	URPGCameraComponent();

	// 启用磁铁或者冰块时的摄像机偏移
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	FVector MagnetSocketOffset;

	// 摄像机偏移的速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MagnetOffsetSpeed = 10.0f;

	// 摄像机复原的速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float OffsetRestoreSpeed = 5.0f;

	// TopBroadView 模式下摄像机 spring arm 的最大长度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpringArmMaxLength = 1000.0f;

	// TopBroadView 模式下摄像机 spring arm 的变化速度
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float SpringArmLengthChangeSpeed = 100.0f;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraMode(ECameraMode NewCameraMode);

	void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void BeginPlay() override;

private:
	ECameraFlags CameraFlags;

	float DefaultSpringArmLength;

	UPROPERTY()
	TObjectPtr<USpringArmComponent> SpringArm; // 用于控制摄像机的弹簧臂组件

	// 在	TopBroadView 模式下更新摄像机长度
	void UpdateSpringArmLength(float DeltaTime);

	// 在 Magnet 模式下更新摄像机偏移或者将其复原
	void UpdateSocketOffset(float DeltaTime);

	void OnCameraModeChanged(EAbilityType, EAbilityStatus PreviousStatus, EAbilityStatus NewStatus);
};
