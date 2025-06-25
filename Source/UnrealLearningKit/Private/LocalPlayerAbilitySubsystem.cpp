// Fill out your copyright notice in the Description page of Project Settings.


#include "LocalPlayerAbilitySubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ULocalPlayerAbilitySubsystem* ULocalPlayerAbilitySubsystem::Get(UWorld* World, int32 PlayerIndex)
{
  if (auto* PC = UGameplayStatics::GetPlayerController(World, PlayerIndex))
  {
    if (ULocalPlayer* LocalPlayer = PC->GetLocalPlayer())
    {
      return LocalPlayer->GetSubsystem<ULocalPlayerAbilitySubsystem>();
    }
  }
  return nullptr;
}




