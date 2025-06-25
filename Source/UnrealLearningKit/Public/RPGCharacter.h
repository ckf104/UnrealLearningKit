// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "UObject/ObjectPtr.h"
#include "RPGCharacter.generated.h"

enum class EAttackState : uint8
{
	Idle,
	Combo1,
	Combo2,
	Combo3,
	Combo4,
	Combo5,
};

UCLASS()
class UNREALLEARNINGKIT_API ARPGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARPGCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Action")
	void TriggerAttack();
	
	UPROPERTY(EditAnywhere, Category = "Action")	
	TObjectPtr<class UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Action")
	TObjectPtr<class UAnimMontage> AttackMontageInAir;


private:
	void StartComboPeriod(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	void EndComboPeriod(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);
	void MontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);


	EAttackState AttackState = EAttackState::Idle;

	bool bComboPeriod = false; // If true, the player can chain attacks in a combo
	bool bComboSuccess = false; // If true, the player successfully performed a combo attack
};
