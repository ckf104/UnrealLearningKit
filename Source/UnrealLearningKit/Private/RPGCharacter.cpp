// Fill out your copyright notice in the Description page of Project Settings.

#include "RPGCharacter.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "CollisionQueryParams.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/EngineTypes.h"
#include "Engine/HitResult.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GenericTeamAgentInterface.h"
#include "KismetTraceUtils.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/AssertionMacros.h"
#include "Runtime/AIModule/Classes/BehaviorTree/BTTaskNode.h"
#include "RPGBlueprintFunctionLibrary.h"

// Sets default values
ARPGCharacter::ARPGCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	HitBoxComponentName = FName("SwordBox");
}

// Called when the game starts or when spawned
void ARPGCharacter::BeginPlay()
{
	Super::BeginPlay();
	auto* MeshComp = GetMesh();
	// TODO: 如何动态地绑定和解绑通知
	// MeshComp->GetAnimInstance()->OnPlayMontageNotifyBegin.AddDynamic(this, &ARPGCharacter ::StartComboPeriod);
	// MeshComp->GetAnimInstance()->OnPlayMontageNotifyEnd.AddDynamic(this, &ARPGCharacter::EndComboPeriod);
	MeshComp->GetAnimInstance()->OnMontageBlendingOut.AddDynamic(this, &ARPGCharacter::MontageBlendingOut);
}

void ARPGCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	// Find the hit box component by name
	ForEachComponent<UBoxComponent>(false,
			[&](UActorComponent* Component) {
				if (Component->GetName() == HitBoxComponentName.ToString())
				{
					HitBox = Cast<UBoxComponent>(Component);
				}
			});
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

void ARPGCharacter::StartComboPeriod()
{
	bComboPeriod = true;
	bComboSuccess = false;
}

void ARPGCharacter::EndComboPeriod()
{
	bComboPeriod = false;
	bComboSuccess = false;
	AttackState = EAttackState::Idle;
}

void ARPGCharacter::JumpNextComboSection()
{
	if (bComboPeriod && bComboSuccess)
	{
		switch (AttackState)
		{
			case EAttackState::Combo1:
				AttackState = EAttackState::Combo2;
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(FName("Combo2"), AttackMontage);
				break;
			case EAttackState::Combo2:
				AttackState = EAttackState::Combo3;
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(FName("Combo3"), AttackMontage);
				break;
			case EAttackState::Combo3:
				AttackState = EAttackState::Combo4;
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(FName("Combo4"), AttackMontage);
				break;
			case EAttackState::Combo4:
				AttackState = EAttackState::Combo5;
				GetMesh()->GetAnimInstance()->Montage_JumpToSection(FName("Combo5"), AttackMontage);
				break;
			default:
				break;
		}
	}
}

void ARPGCharacter::MontageBlendingOut(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage == AttackMontage)
	{
		bComboPeriod = false;
		bComboSuccess = false;
		AttackState = EAttackState::Idle;
	}
}

void ARPGCharacter::TriggerAttack()
{
	if (AttackState == EAttackState::Idle)
	{
		PlayAnimMontage(AttackMontage);
		AttackState = EAttackState::Combo1;
	}
	else if (AttackState != EAttackState::Hit)
	{
		bComboSuccess = bComboSuccess | bComboPeriod;
	}
}

float ARPGCharacter::InternalTakePointDamage(float Damage, const FPointDamageEvent& PointDamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	bool bIsFalling = GetCharacterMovement()->IsFalling();
	if (bIsFalling)
	{
		return Damage;
	}

	FVector HitLocation = PointDamageEvent.HitInfo.ImpactPoint;
	auto HitDir = URPGBlueprintFunctionLibrary::GetHitDirection(HitLocation, this);
	if (bDrawSword)
	{
		// Play the hit montage based on the hit direction
		PlayAnimMontage(HitMontagesWithSword[HitDir]);
	}
	else
	{
		PlayAnimMontage(HitMontagesWithoutSword[HitDir]);
	}
	UE_LOG(LogTemp, Log, TEXT("Hit by %s with damage %f, hit direction: %d"), *DamageCauser->GetName(), Damage, static_cast<int32>(HitDir));
	AttackState = EAttackState::Hit;
	return Damage;
}

void ARPGCharacter::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	// If not idle, ignore movement input
	if (AttackState != EAttackState::Idle)
	{
		return;
	}
	// if (AnimState != EAttackState::Idle)
	{
		// If not idle, interrupt the current animation
		auto& AllMontageInstance = GetMesh()->GetAnimInstance()->MontageInstances;
		for (auto* MontageInstance : AllMontageInstance)
		{
			// TODO：更好的办法来打断攻击，受击蒙太奇
			// TODO：确定 montage 剩余的播放时间，和 interrupt blending time 取最小值
			if (MontageInstance && MontageInstance->IsActive() && MontageInstance->Montage->GetGroupName() == FName("DefaultGroup"))
			{
				if (MontageInstance->Montage == AttackMontage || MontageInstance->Montage == AttackMontageInAir || IsHitMontage(MontageInstance->Montage))
				{
					// If the montage is an attack montage, stop it with the interrupt blend time
					GetMesh()->GetAnimInstance()->Montage_Stop(InterruptMontageBlendTime, MontageInstance->Montage);
				}
			}
		}
	}

	Super::AddMovementInput(WorldDirection, ScaleValue, bForce);
}

bool ARPGCharacter::IsHitMontage(const class UAnimMontage* Montage) const
{
	for (const auto& HitMontage : HitMontagesWithSword)
	{
		if (HitMontage.Value == Montage)
		{
			return true;
		}
	}
	for (const auto& HitMontage : HitMontagesWithoutSword)
	{
		if (HitMontage.Value == Montage)
		{
			return true;
		}
	}
	return false;
}

void ARPGCharacter::NotifyAttackBegin()
{
	ensure(LastWeaponPos.IsZero());
	LastWeaponPos = HitBox->GetComponentLocation();
}

void ARPGCharacter::NotifyAttackTick()
{
	auto* World = GetWorld();
	if (World && HitBox)
	{
		auto NewWeaponPos = HitBox->GetComponentLocation();
		TArray<FHitResult> HitResults;
		auto QueryParams = FCollisionObjectQueryParams::DefaultObjectQueryParam;
		QueryParams.AddObjectTypesToQuery(ECollisionChannel::ECC_Pawn);
		auto bHit = World->SweepMultiByObjectType(HitResults, LastWeaponPos, NewWeaponPos, HitBox->GetComponentRotation().Quaternion(), QueryParams, FCollisionShape::MakeBox(HitBox->GetScaledBoxExtent()));
		if (bDebugDrawHitBox)
		{
			DrawDebugBoxTraceMulti(World, LastWeaponPos, NewWeaponPos, HitBox->GetScaledBoxExtent(), HitBox->GetComponentRotation(), EDrawDebugTrace::Type::ForDuration, bHit, HitResults, FLinearColor::Green, FLinearColor::Red, 5.0f);
		}
		for (const FHitResult& HitResult : HitResults)
		{
			auto* Actor = HitResult.GetActor();
			if (Actor && !HitActors.Contains(Actor))
			{
				if (this->GetTeamAttitudeTowards(*Actor) == ETeamAttitude::Hostile)
				{
					HitActors.Add(Actor);
					auto PointDamage = FPointDamageEvent();
					PointDamage.Damage = AttackPower;
					PointDamage.HitInfo = HitResult;
					Actor->TakeDamage(AttackPower, PointDamage, GetController(), this);
				}
			}
			// UE_LOG(LogTemp, Warning, TEXT("%s Hit %s with %f damage."), *this->GetName(), *HitResult.Actor->GetName(), AttackPower);
		}
		LastWeaponPos = NewWeaponPos;
	}
}

void ARPGCharacter::NotifyAttackEnd()
{
	// Clear the hit actors after the attack ends
	HitActors.Empty();
	// Reset the weapon position
	LastWeaponPos = FVector::ZeroVector;
}
