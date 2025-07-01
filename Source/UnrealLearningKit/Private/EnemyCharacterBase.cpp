// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyCharacterBase.h"
#include "AIController.h"
#include "Components/PrimitiveComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/DamageEvents.h"
#include "GenericTeamAgentInterface.h"
#include "AttackUIInterface.h"
#include "RPGCharacter.h"
#include "TimerManager.h"

// Sets default values
AEnemyCharacterBase::AEnemyCharacterBase()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	HitActorClass = ARPGCharacter::StaticClass();
	HitBoxComponentName = TEXT("Attack1Box");
	HPWidgetName = TEXT("HPWidget");
}

// Called when the game starts or when spawned
void AEnemyCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	// widget component 在 begin play 的时候才会创建 user widget
	ForEachComponent<UWidgetComponent>(false,
			[&](UActorComponent* Component) {
				if (Component->GetName() == HPWidgetName)
				{
					auto* WidgetComp = Cast<UWidgetComponent>(Component);
					if (WidgetComp)
					{
						HPWidgetComp = WidgetComp->GetUserWidgetObject();
					}
				}
			});
}

FGenericTeamId AEnemyCharacterBase::GetGenericTeamId() const
{
	// Assuming enemies belong to a specific team, e.g., Team 1
	auto* ActorOwner = Cast<AAIController>(GetOwner());
	if (ActorOwner)
	{
		return ActorOwner->GetGenericTeamId();
	}
	return FGenericTeamId::NoTeam;
}

// Called every frame
void AEnemyCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AEnemyCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyCharacterBase::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	ForEachComponent<UPrimitiveComponent>(false,
			[&](UActorComponent* Component) {
				if (Component->GetName() == HitBoxComponentName)
				{
					HitBox = Cast<UPrimitiveComponent>(Component);
				}
			});
}

void AEnemyCharacterBase::DealAttackEvent_Implementation()
{
	if (HitBox)
	{
		TArray<AActor*> HitActors;
		HitBox->GetOverlappingActors(HitActors, HitActorClass);
		for (AActor* HitActor : HitActors)
		{
			if (HitActor && HitActor->IsValidLowLevel())
			{
				auto PointDamage = FPointDamageEvent();
				// 好像没办法从非 root 的 primitive component 中获取详细的 overlap 信息
				PointDamage.HitInfo.ImpactPoint = HitBox->GetComponentLocation();
				HitActor->TakeDamage(AttackAttr.AttackPower, PointDamage, GetController(), this);
				// UE_LOG(LogTemp, Warning, TEXT("%s Hit %s with %f damage."), *this->GetName(), *HitActor->GetName(), AttackPower);
			}
		}
	}
}

float AEnemyCharacterBase::CalculateRealDamage(float Damage) const
{
	// Calculate real damage based on defense power
	auto RealDamage = FMath::Max(Damage - AttackAttr.DefensePower, 0.0f);
	return RealDamage;
}

float AEnemyCharacterBase::InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	UE_LOG(LogTemp, Log, TEXT("Enemy %s took %f damage from %s"), *GetName(), Damage, *DamageCauser->GetName());
	auto RealDamage = CalculateRealDamage(Damage);
	AttackAttr.CurrentHP = FMath::Clamp(AttackAttr.CurrentHP - RealDamage, 0.0f, AttackAttr.TotalHP);
	UINotifyWhenTakeDamage(RealDamage);
	if (AttackAttr.CurrentHP <= 0.0f)
	{
		// TODO something
	}
	return RealDamage;
}

void AEnemyCharacterBase::ShowHP(bool bShow)
{
	if (HPWidgetComp)
	{
		HPWidgetComp->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		if (bShow)
		{
			// Set timer to hide HP after 2 seconds
			GetWorldTimerManager().ClearTimer(HideHPHandle);
			GetWorldTimerManager().SetTimer(HideHPHandle, FTimerDelegate::CreateUObject(this, &AEnemyCharacterBase::ShowHP, false), 2.0f, false);
		}
		else
		{
			GetWorldTimerManager().ClearTimer(HideHPHandle);
		}
	}
}

void AEnemyCharacterBase::UINotifyWhenTakeDamage(float RealDamage)
{
	if (HPWidgetComp)
	{
		auto* AttackUIInterface = Cast<IAttackUIInterface>(HPWidgetComp);
		if (HPWidgetComp.GetClass()->ImplementsInterface(UAttackUIInterface::StaticClass()))
		{
			IAttackUIInterface::Execute_SetHPPercentage(HPWidgetComp, FMath::Clamp(AttackAttr.CurrentHP / AttackAttr.TotalHP, 0.0f, 1.0f));
		}
	}
	ShowHP(true);
	ShowDamageNumber(RealDamage);
}

void AEnemyCharacterBase::ShowDamageNumber(float RealDamage)
{
	// TODO
}
