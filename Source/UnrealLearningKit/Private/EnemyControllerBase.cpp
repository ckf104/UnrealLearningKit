// Fill out your copyright notice in the Description page of Project Settings.

#include "EnemyControllerBase.h"

AEnemyControllerBase::AEnemyControllerBase(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
{
	SetGenericTeamId(FGenericTeamId((uint8)RPGTeam::Enemy));
}

void AEnemyControllerBase::UpdateFocus(AActor* NewFocus, const FAIStimulus& Stimulus)
{
	if (Stimulus.WasSuccessfullySensed())
	{
		if (GetFocusActorForPriority(EAIFocusPriority::Gameplay))
		{
			return;
		}
		SetFocus(NewFocus, EAIFocusPriority::Gameplay);
	}
	else
	{
		if (GetFocusActorForPriority(EAIFocusPriority::Gameplay) == NewFocus)
		{
			ClearFocus(EAIFocusPriority::Gameplay);
		}
	}
}