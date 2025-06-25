// Fill out your copyright notice in the Description page of Project Settings.


#include "MagnetHandleComponent.h"
#include "PhysicsInterfaceDeclaresCore.h"
#include "PhysicsProxy/SingleParticlePhysicsProxy.h"

bool UMagnetHandleComponent::GetPhysicsTransform(FVector& Location, FRotator& Rotation) const
{
  if (KinematicHandle)
  {
    FPhysicsCommand::ExecuteRead(GrabbedHandle, [&](const FPhysicsActorHandle& InKinematicHandle)
    {
      // auto Transform = FPhysicsInterface::GetGlobalPose_AssumesLocked(InKinematicHandle);
      // Location = Transform.GetTranslation();
      // Rotation = Transform.Rotator();
      Location = InKinematicHandle->GetGameThreadAPI().GetX();
      Rotation = InKinematicHandle->GetGameThreadAPI().GetR().Rotator();
      // InKinematicHandle->GetGameThreadAPI().GetW();
      // InKinematicHandle->GetGameThreadAPI().GetV();
    });
    return true;
  }
  return false;
}

bool UMagnetHandleComponent::GetKinematicTransform(FVector& Location, FRotator& Rotation) const
{
  if (KinematicHandle)
  {
    FPhysicsCommand::ExecuteRead(KinematicHandle, [&](const FPhysicsActorHandle& InKinematicHandle)
    {
      auto Transform = FPhysicsInterface::GetKinematicTarget_AssumesLocked(InKinematicHandle);
      Location = Transform.GetTranslation();
      Rotation = Transform.Rotator();
    });
    return true;
  }
  return false;
}


