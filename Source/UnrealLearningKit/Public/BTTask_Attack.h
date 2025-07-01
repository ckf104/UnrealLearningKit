// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Attack.generated.h"

/**
 * 
 */
UCLASS()
class UNREALLEARNINGKIT_API UBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "Action")
	TObjectPtr<class UAnimMontage> AttackMontage;

	UPROPERTY(EditAnywhere, Category = "Action")
	float PlayRate = 1.0f;

	UBTTask_Attack();	
	
	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;	

	uint16 GetInstanceMemorySize() const override { return 0; }

private:
	// TODO: 是否安全?
	void MontageEnded(UAnimMontage* Montage, bool bInterrupted, class UBehaviorTreeComponent* OwnerComp);
};
