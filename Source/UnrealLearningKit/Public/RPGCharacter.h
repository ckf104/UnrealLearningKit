// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Animation/AnimNotifies/AnimNotify.h"
#include "AttackInterface.h"
#include "CoreMinimal.h"
#include "EnemyControllerBase.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "UObject/ObjectPtr.h"
#include "RPGCharacter.generated.h"

UENUM(BlueprintType)
enum class EAttackState : uint8
{
	Idle,
	Hit,
	Combo1,
	Combo2,
	Combo3,
	Combo4,
	Combo5,
	Dead,
};

UENUM(BlueprintType)
enum class HitDirection : uint8
{
	Front,
	Back,
	Left,
	Right,
};

UCLASS()
class UNREALLEARNINGKIT_API ARPGCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ARPGCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	FGenericTeamId GetGenericTeamId() const override { return FGenericTeamId((uint8)RPGTeam::Player); }

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Action")
	void TriggerAttack();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void StartComboPeriod();
	UFUNCTION(BlueprintCallable, Category = "Action")
	void EndComboPeriod();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void JumpNextComboSection();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void NotifyAttackBegin();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void NotifyAttackTick();

	UFUNCTION(BlueprintCallable, Category = "Action")
	void NotifyAttackEnd();

	UFUNCTION(BlueprintCallable, Category = "Action")
	bool IsIdle() const { return AttackState == EAttackState::Idle; }
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	bool bDrawSword = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float InterruptMontageBlendTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float AttackPower = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Action")	
	TObjectPtr<class UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Action")
	TObjectPtr<class UAnimMontage> AttackMontageInAir;

	UPROPERTY(EditAnywhere, Category = "Action")
	TMap<HitDirection, TObjectPtr<UAnimMontage>> HitMontagesWithSword;

	UPROPERTY(EditAnywhere, Category = "Action")
	TMap<HitDirection, TObjectPtr<UAnimMontage>> HitMontagesWithoutSword;

	UPROPERTY(EditAnywhere, Category = "Action")
	FName HitBoxComponentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	TObjectPtr<class UBoxComponent> HitBox;

	void AddMovementInput(FVector WorldDirection, float ScaleValue = 1.0f, bool bForce = false) override;

	void PostInitializeComponents() override;

private:

	UFUNCTION()
	void MontageBlendingOut(UAnimMontage* Montage, bool bInterrupted);

	bool IsHitMontage(const class UAnimMontage*) const;

	UPROPERTY()
	TSet<class AActor*> HitActors; // Set of actors hit by the character, used to prevent multiple hits on the same actor in a single attack

	FVector LastWeaponPos;

	// 当前的逻辑状态
	UPROPERTY(BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	EAttackState AttackState = EAttackState::Idle;

	// 当前的动画状态
	// UPROPERTY(BlueprintReadOnly, Category = "Action", meta = (AllowPrivateAccess = "true"))
	// EAttackState AnimState = EAttackState::Idle;

	bool bComboPeriod = false; // If true, the player can chain attacks in a combo
	bool bComboSuccess = false; // If true, the player successfully performed a combo attack

protected:
	float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	UPROPERTY(EditAnywhere, Category = "Debug")
	bool bDebugDrawHitBox = false; // If true, draw the hit box for debugging purposes
};
