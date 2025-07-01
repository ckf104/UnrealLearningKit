// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AttackInterface.h"
#include "CoreMinimal.h"
#include "Engine/TimerHandle.h"
#include "GameFramework/Character.h"
#include "GameFramework/Pawn.h"
#include "GenericTeamAgentInterface.h"
#include "Templates/SubclassOf.h"

#include "EnemyCharacterBase.generated.h"

USTRUCT(BlueprintType, Blueprintable)
struct FAttackAttr
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float AttackPower = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float DefensePower = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float TotalHP = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	float CurrentHP = 100.0f;
};

UCLASS()
class UNREALLEARNINGKIT_API AEnemyCharacterBase : public ACharacter, public IAttackInterface, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemyCharacterBase();

	FGenericTeamId GetGenericTeamId() const override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	struct FAttackAttr AttackAttr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	FName HitBoxComponentName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Action")
	TObjectPtr<class UPrimitiveComponent> HitBox;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Action")
	TSubclassOf<AActor> HitActorClass;

	void PostInitializeComponents() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Implement the Attack Interface
	void DealAttackEvent_Implementation() override;

	float InternalTakePointDamage(float Damage, struct FPointDamageEvent const& PointDamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	float CalculateRealDamage(float Damage) const;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FName HPWidgetName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
	TObjectPtr<class UUserWidget> HPWidgetComp;

private:
	UPROPERTY(EditAnywhere, Category = "UI", meta = (AllowPrivateAccess = "true"))
	float ShowHPTime = 2.0f; // Time to show HP bar

	FTimerHandle HideHPHandle;

	void UINotifyWhenTakeDamage(float RealDamage);
	void ShowHP(bool bShow);
	void ShowDamageNumber(float RealDamage);
};
