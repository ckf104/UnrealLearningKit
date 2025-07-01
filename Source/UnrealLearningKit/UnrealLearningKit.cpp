// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealLearningKit.h"
#include "GenericTeamAgentInterface.h"
#include "Modules/ModuleManager.h"
#include "Public/EnemyControllerBase.h"

class FRPGGameModule : public FDefaultGameModuleImpl
{
public:
	void StartupModule() override
	{
		FGenericTeamId::SetAttitudeSolver([](FGenericTeamId A, FGenericTeamId B) {
			if (A.GetId() == (uint8)RPGTeam::Enemy && B.GetId() == (uint8)RPGTeam::Player)
			{
				return ETeamAttitude::Hostile;
			}
			else if (A.GetId() == (uint8)RPGTeam::Player && B.GetId() == (uint8)RPGTeam::Enemy)
			{
				return ETeamAttitude::Hostile;
			}
			else if (A == B && (A.GetId() == (uint8)RPGTeam::Player || A.GetId() == (uint8)RPGTeam::Enemy))
			{
				return ETeamAttitude::Friendly;
			}
			return ETeamAttitude::Neutral;
		});
	}
};

IMPLEMENT_PRIMARY_GAME_MODULE(FRPGGameModule, UnrealLearningKit, "UnrealLearningKit");
