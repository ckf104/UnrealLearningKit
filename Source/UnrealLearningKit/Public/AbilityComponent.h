

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/HitResult.h"
#include "HAL/Platform.h"
#include "AbilityComponent.generated.h"


UCLASS(Abstract, Blueprintable)
class UNREALLEARNINGKIT_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAbilityComponent();

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	uint8 bDrawTraceLine : 1;
	

protected:

	UPROPERTY(BlueprintReadWrite)
	uint8 bEnable : 1;

	UFUNCTION(BlueprintCallable)
	FHitResult LineTraceFromCamera(float maxDistance) const;

public:	

	UFUNCTION(BlueprintImplementableEvent, BlueprintPure)
	bool CanUse() const;
};
