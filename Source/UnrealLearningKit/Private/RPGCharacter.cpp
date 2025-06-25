// Fill out your copyright notice in the Description page of Project Settings.

#include "RPGCharacter.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"

// Sets default values
ARPGCharacter::ARPGCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ARPGCharacter::BeginPlay()
{
	Super::BeginPlay();
	auto* MeshComp = GetMesh();
	// TODO: 如何动态地绑定和解绑通知
	MeshComp->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ARPGCharacter ::StartComboPeriod);
	MeshComp->GetAnimInstance()->OnPlayMontageNotifyEnd.AddDynamic(this, &ARPGCharacter::EndComboPeriod);
	MeshComp->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &ARPGCharacter::MontageBlendingOut);
}

// Called every frame
void ARPGCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ARPGCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ARPGCharacter::StartComboPeriod(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	bComboPeriod = true;
	bComboSuccess = false;
}

void ARPGCharacter::EndComboPeriod(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	bComboPeriod = false;
	bComboSuccess = false;
	AttackState = EAttackState::Idle;
}

void ARPGCharacter::MontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	bComboPeriod = false;
	bComboSuccess = false;
	AttackState = EAttackState::Idle;
}

void ARPGCharacter::TriggerAttack()
{
	if (AttackState == EAttackState::Idle)
	{
		AttackState = EAttackState::Combo1;
		// TODO: Play Montage
	}
	else
	{
		bComboSuccess = bComboSuccess | bComboPeriod;
		if (bComboPeriod)
		{
			switch (AttackState)
			{
				case EAttackState::Combo1:
					AttackState = EAttackState::Combo2;
					break;
				case EAttackState::Combo2:
					AttackState = EAttackState::Combo3;
					break;
				case EAttackState::Combo3:
					AttackState = EAttackState::Combo4;
					break;
			}
		}
	}
}
