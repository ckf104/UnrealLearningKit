// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_Attack.h"
#include "AIController.h"
#include "Animation/AnimInstance.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

UBTTask_Attack::UBTTask_Attack()
{
	NodeName = "Attack";
	INIT_TASK_NODE_NOTIFY_FLAGS();

	// BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, BlackboardKey), AActor::StaticClass());
}

void UBTTask_Attack::MontageEnded(UAnimMontage* Montage, bool bInterrupted, class UBehaviorTreeComponent* OwnerComp)
{
	FinishLatentTask(*OwnerComp, EBTNodeResult::Type::Succeeded);
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	if (!AttackMontage)
	{
		return EBTNodeResult::Failed;
	}
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}
	auto* Character = Cast<ACharacter>(AIController->GetPawn());
	if (!Character)
	{
		return EBTNodeResult::Failed;
	}
	Character->PlayAnimMontage(AttackMontage, PlayRate);
	auto* AnimInstance = Character->GetMesh()->GetAnimInstance();
	auto Delegate = FOnMontageEnded::CreateUObject(this, &UBTTask_Attack::MontageEnded, &OwnerComp);
	AnimInstance->Montage_SetEndDelegate(Delegate, AttackMontage);
	return EBTNodeResult::InProgress;
}
