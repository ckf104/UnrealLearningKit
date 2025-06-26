// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTask_Attack.h"


UBTTask_Attack::UBTTask_Attack()
{
  NodeName = "Attack";
  INIT_TASK_NODE_NOTIFY_FLAGS();

  // BlackboardKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_Attack, BlackboardKey), AActor::StaticClass());
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
  return EBTNodeResult::Succeeded; 
}

