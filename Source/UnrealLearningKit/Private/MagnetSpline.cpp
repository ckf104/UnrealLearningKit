// Fill out your copyright notice in the Description page of Project Settings.

#include "MagnetSpline.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "CoreGlobals.h"
#include "Engine/EngineTypes.h"
#include "Math/Color.h"
#include "Math/MathFwd.h"
#include "Math/RotationMatrix.h"
#include "Math/UnrealMathUtility.h"
#include "Misc/AssertionMacros.h"
#include "Templates/SubclassOf.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealNames.h"

// Sets default values
AMagnetSpline::AMagnetSpline()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bAllowTickBeforeBeginPlay = false;
	Spline = CreateDefaultSubobject<USplineComponent>(TEXT("Spline"));
	RootComponent = Spline;

	Fat = { 1.0f, 1.0f };
	StaticMeshLength = 100.0f;
	DesiredMeshNumber = 3;

	Amplitudes = { 1.0f, 1.0f };
	Frequencies = { 5.0f, 3.0f };
	Phases = { 1.0f, 2.0f };
	InputKeys = { 1, 2 };
	LengthRatios = { 0.3, 0.5 };
	InterpSpeed = { 5, 5 };
	Material = { nullptr, nullptr, nullptr };
	bUseKeyFrames = false;
	AnimSplineBaseLength = 1000.0f;

	TotalTime = 0.0f;
}

// Called when the game starts or when spawned
void AMagnetSpline::BeginPlay()
{
	// 我们为每个 spline mesh 单独设置它的 collision
	// SetActorEnableCollision(false);
	Super::BeginPlay();
	SetInitialPosition();
}

double AMagnetSpline::GetSwingStrengthForSplinePoint_Implementation(int32 SplinePointIndex, double TotalLength, FVector ControlDir)
{
	return 1.0;
}

double AMagnetSpline::GetXMoveStrengthForSplinePoint_Implementation(int32 SplinePointIndex, double TotalLength, FVector ControlDir)
{
	return 1.0;
}

double AMagnetSpline::GetZMoveStrengthForSplinePoint_Implementation(int32 SplinePointIndex, double TotalLength, FVector ControlDir)
{
	return 1.0;
}

double AMagnetSpline::GetTangentStrengthForSplinePoint_Implementation(int32 SplinePointIndex, double TotalLength, FVector ControlDir)
{
	return 1.0;
}

FVector AMagnetSpline::GetStartAttachPos()
{
	auto StartLoc = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	if (StartAttachedComp)
	{
		if (StartAttachedComp->IsSimulatingPhysics())
		{
			StartLoc = StartAttachedComp->GetCenterOfMass();
		}
		else
		{
			StartLoc = StartAttachedComp->GetComponentLocation() + StartPosOffset;
		}
	}
	return StartLoc;
}

FVector AMagnetSpline::GetEndAttachPos()
{
	auto EndLoc = Spline->GetLocationAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, ESplineCoordinateSpace::World);
	if (EndAttachedComp)
	{
		if (EndAttachedComp->IsSimulatingPhysics())
		{
			EndLoc = EndAttachedComp->GetCenterOfMass();
		}
		else
		{
			EndLoc = EndAttachedComp->GetComponentLocation() + EndPosOffset;
		}
	}
	return EndLoc;
}

// 设置初始位置
void AMagnetSpline::SetInitialPosition()
{
	if (!bUseKeyFrames)
	{
		auto StartLoc = GetStartAttachPos();
		Spline->SetLocationAtSplinePoint(0, StartLoc, ESplineCoordinateSpace::World, false);
		auto EndLoc = GetEndAttachPos();
		Spline->SetLocationAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, EndLoc, ESplineCoordinateSpace::World, false);
		for (int32 i = 0; i < InputKeys.Num(); ++i)
		{
			auto VertexLoc = FMath::Lerp(StartLoc, EndLoc, LengthRatios[i]);
			Spline->SetLocationAtSplinePoint(InputKeys[i], VertexLoc, ESplineCoordinateSpace::World, false);
		}
	}
	// TODO：Tangent 记得插值
}

// 计算每个 mesh 的位置
void AMagnetSpline::SwingPositionUpdate(float DeltaTime)
{
	// DumpSplinePosition();

	auto StartLoc = GetStartAttachPos();
	Spline->SetLocationAtSplinePoint(0, StartLoc, ESplineCoordinateSpace::World, false);
	auto EndLoc = GetEndAttachPos();
	Spline->SetLocationAtSplinePoint(Spline->GetNumberOfSplinePoints() - 1, EndLoc, ESplineCoordinateSpace::World, false);

	auto Length = (EndLoc - StartLoc).Size();
	auto XAxis = (EndLoc - StartLoc).GetSafeNormal();
	auto ZAxis = FVector::UpVector;
	auto Coords = FRotationMatrix::MakeFromXZ(XAxis, ZAxis);

	auto RightDir = Coords.GetScaledAxis(EAxis::Y);
	ControlMoveDir.Y = FMath::Clamp(ControlMoveDir.Y, -1.0f, 1.0f);
	ControlMoveDir.Z = FMath::Clamp(ControlMoveDir.Z, -1.0f, 1.0f);
	for (int32 i = 0; i < InputKeys.Num(); ++i)
	{
		auto VertexLoc = FMath::Lerp(StartLoc, EndLoc, LengthRatios[i]);
		auto Delta = Amplitudes[i] * FMath::Sin(Frequencies[i] * TotalTime + Phases[i]);
		auto DeltaLoc1 = RightDir * Delta;
		auto SwingStrength = GetSwingStrengthForSplinePoint(InputKeys[i], Length, ControlMoveDir);
		DeltaLoc1 *= SwingStrength;

		auto XMoveStrength = GetXMoveStrengthForSplinePoint(InputKeys[i], Length, ControlMoveDir);
		auto ZMoveStrength = GetZMoveStrengthForSplinePoint(InputKeys[i], Length, ControlMoveDir);
		auto XMoveDelta = XMoveStrength * ControlMoveDir.Y * RightDir.GetSafeNormal2D();
		auto ZMoveDelta = ZMoveStrength * ControlMoveDir.Z * ZAxis;
		auto DeltaLoc2 = XMoveDelta + ZMoveDelta;
		if (i == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("StartLoc: %s, EndLoc: %s, X move delta: %s, Z move delta: %s"), *StartLoc.ToString(), *EndLoc.ToString(), *XMoveDelta.ToString(), *ZMoveDelta.ToString());
		}

		auto TargetPos = VertexLoc + DeltaLoc1 + DeltaLoc2;
		auto LastPos = Spline->GetLocationAtSplinePoint(InputKeys[i], ESplineCoordinateSpace::World);
		auto NewPos = FMath::VInterpTo(LastPos, TargetPos, DeltaTime, InterpSpeed[i]);

		Spline->SetLocationAtSplinePoint(InputKeys[i], NewPos, ESplineCoordinateSpace::World, false);
	}
	ControlMoveDir = FVector::ZeroVector;
}

void AMagnetSpline::SplineTangentUpdate()
{
	auto StartLoc = Spline->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::World);
	auto NumPoints = Spline->SplineCurves.Position.Points.Num();
	auto EndLoc = Spline->GetLocationAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::World);

	auto Dir = (EndLoc - StartLoc).GetSafeNormal();
	auto Length = (EndLoc - StartLoc).Size();

	for (int32 i = 0; i < InputKeys.Num(); ++i)
	{
		// auto Tangent = Spline->GetTangentAtSplinePoint(InputKeys[i], ESplineCoordinateSpace::World);
		auto Strength = GetTangentStrengthForSplinePoint(InputKeys[i], Length, ControlMoveDir);
		auto Tangent = Strength * Dir;
		Spline->SetTangentAtSplinePoint(i, Tangent, ESplineCoordinateSpace::World, false);
	}

	auto FirstKeyPos = Spline->GetLocationAtSplinePoint(InputKeys[0], ESplineCoordinateSpace::World);
	auto StartToFirstKeyLength = (FirstKeyPos - StartLoc).Size();
	auto MidPos = FirstKeyPos - Dir * StartToFirstKeyLength * 0.5f;
	auto FirstTangent = (MidPos - StartLoc).GetSafeNormal();
	FirstTangent *= GetTangentStrengthForSplinePoint(0, Length, ControlMoveDir);
	Spline->SetTangentAtSplinePoint(0, FirstTangent, ESplineCoordinateSpace::World, false);

	auto LastKeyPos = Spline->GetLocationAtSplinePoint(InputKeys[InputKeys.Num() - 1], ESplineCoordinateSpace::World);
	auto EndToLastKeyLength = (EndLoc - LastKeyPos).Size();
	auto MidPos2 = LastKeyPos + Dir * EndToLastKeyLength * 0.5f;
	auto LastTangent = (MidPos2 - EndLoc).GetSafeNormal();
	LastTangent *= GetTangentStrengthForSplinePoint(NumPoints - 1, Length, ControlMoveDir);
	Spline->SetTangentAtSplinePoint(NumPoints - 1, LastTangent, ESplineCoordinateSpace::World, false);
}

int32 AMagnetSpline::GetKeyframeIndexAndBlendAlpha(float& Alpha)
{
	// 我们要求 loop anim
	auto KeyframeTotalTime = KeyFrames[KeyFrames.Num() - 1].Time;
	auto KeyframeTime = FMath::Fmod(TotalTime, KeyframeTotalTime);

	for (int32 i = 0; i < KeyFrames.Num() - 1; ++i)
	{
		if (KeyframeTime >= KeyFrames[i].Time && KeyframeTime <= KeyFrames[i + 1].Time)
		{
			auto TimeDiff = KeyFrames[i + 1].Time - KeyFrames[i].Time;
			Alpha = (KeyFrames[i + 1].Time - KeyframeTime) / TimeDiff;
			return i;
		}
	}
	ensure(0);
	return -1;
}

bool AMagnetSpline::KeyframeCheck()
{
	if (KeyFrames.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Keyframe number is less than 2"));
		return false;
	}
	for (int32 i = 0; i < KeyFrames.Num(); ++i)
	{
		if (KeyFrames[i].Position.Num() != Spline->GetNumberOfSplinePoints())
		{
			UE_LOG(LogTemp, Error, TEXT("Keyframe %d position number is not equal to spline point number"), i);
			return false;
		}
		if (KeyFrames[i].Tangent.Num() != Spline->GetNumberOfSplinePoints())
		{
			UE_LOG(LogTemp, Error, TEXT("Keyframe %d tangent number is not equal to spline point number"), i);
			return false;
		}
		if (i > 0 && KeyFrames[i].Time <= KeyFrames[i - 1].Time)
		{
			UE_LOG(LogTemp, Error, TEXT("Keyframe %d time is not greater than keyframe %d time"), i, i - 1);
			return false;
		}
	}
	return true;
}

FVector AMagnetSpline::Interp(const FVector& Start, const FVector& End, float Alpha)
{
	auto InterpAlpha = Alpha;
	if (bUseCubicInterp)
	{
		InterpAlpha = FMath::CubicInterp(0.0f, CubicStartTangent, 1.0f, CubicEndTangent, Alpha);
	}
	return FMath::Lerp(Start, End, Alpha);
}


void AMagnetSpline::KeyframeSplineUpdate()
{
	if (!KeyframeCheck())
	{
		return;
	}
	// DumpSplinePosition();
	
	auto StartLoc = GetStartAttachPos();
	SetActorLocation(StartLoc);

	auto EndLoc = GetEndAttachPos();
	auto Length = (EndLoc - StartLoc).Size();
	auto XScale = Length / AnimSplineBaseLength;
	Spline->SetRelativeScale3D(FVector(XScale, 1.0f, 1.0f));

	auto XAxis = (EndLoc - StartLoc).GetSafeNormal();
	auto ZAxis = FVector::UpVector;
	auto Coords = FRotationMatrix::MakeFromXZ(XAxis, ZAxis);
	Spline->SetRelativeRotation(Coords.Rotator());

	float FirstFrameAlpha;
	auto FirstFrameIndex = GetKeyframeIndexAndBlendAlpha(FirstFrameAlpha);
	auto SecondFrameIndex = FirstFrameIndex + 1;
	for (int32 i = 0; i < Spline->GetNumberOfSplinePoints(); ++i)
	{
		auto FirstFramePos = KeyFrames[FirstFrameIndex].Position[i];
		auto SecondFramePos = KeyFrames[SecondFrameIndex].Position[i];
		auto FirstFrameTangent = KeyFrames[FirstFrameIndex].Tangent[i];
		auto SecondFrameTangent = KeyFrames[SecondFrameIndex].Tangent[i];

		auto NewLoc = Interp(FirstFramePos, SecondFramePos, 1- FirstFrameAlpha);
		auto NewTangent = Interp(FirstFrameTangent, SecondFrameTangent, 1 - FirstFrameAlpha);

		Spline->SetLocationAtSplinePoint(i, NewLoc, ESplineCoordinateSpace::Local, false);
		Spline->SetTangentAtSplinePoint(i, NewTangent, ESplineCoordinateSpace::Local, false);
	}
	// TODO：增加移动偏移的支持
}

// Called every frame
void AMagnetSpline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bUseKeyFrames)
	{
		KeyframeSplineUpdate();
	}
	else
	{
		SwingPositionUpdate(DeltaTime);
		SplineTangentUpdate();
	}

	TotalTime += DeltaTime;
	Spline->UpdateSpline();

	SetupSplineMeshes();
}

void AMagnetSpline::SetupSplineMeshes()
{
	TArray<USplineMeshComponent*> SplineMeshComponents;
	GetComponents(USplineMeshComponent::StaticClass(), SplineMeshComponents);

	Spline->GetLocationAtSplineInputKey(1.0, ESplineCoordinateSpace::Local);
	DesiredMeshNumber = InputKeys.Num() + 1;

	// auto Scale = ConstructScale(Fat, MeshLengthScale);
	for (int32 i = 0; i < DesiredMeshNumber; ++i)
	{
		USplineMeshComponent* SplineMesh = nullptr;
		if (i < SplineMeshComponents.Num())
		{
			SplineMesh = SplineMeshComponents[i];
		}
		else
		{
			SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->RegisterComponent();
			SplineMesh->SetStaticMesh(StaticMesh);
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->SetMaterial(0, Material[i]);
			SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SplineMesh->ForwardAxis = ForwardAxis;
		}

		FVector StartLoc, StartTangent;
		if (i > 0)
		{
			StartLoc = Spline->GetLocationAtSplineInputKey(float(InputKeys[i - 1]), ESplineCoordinateSpace::Local);
			StartTangent = Spline->GetTangentAtSplineInputKey(float(InputKeys[i - 1]), ESplineCoordinateSpace::Local);
		}
		else
		{
			StartLoc = Spline->GetLocationAtSplineInputKey(0.f, ESplineCoordinateSpace::Local);
			StartTangent = Spline->GetTangentAtSplineInputKey(0.f, ESplineCoordinateSpace::Local);
		}
		FVector EndLoc, EndTangent;
		if (i < DesiredMeshNumber - 1)
		{
			EndLoc = Spline->GetLocationAtSplineInputKey(InputKeys[i], ESplineCoordinateSpace::Local);
			EndTangent = Spline->GetTangentAtSplineInputKey(InputKeys[i], ESplineCoordinateSpace::Local);
		}
		else
		{
			// 没找到 spline 中返回最大 key 的 API，就使用一个超大的 key 值来替代了
			EndLoc = Spline->GetLocationAtSplineInputKey(1000.f, ESplineCoordinateSpace::Local);
			EndTangent = Spline->GetTangentAtSplineInputKey(1000.f, ESplineCoordinateSpace::Local);
		}

		SplineMesh->SetStartAndEnd(StartLoc, StartTangent, EndLoc, EndTangent, false);

		// TODO：解释为什么设置 Scale 来改变 spline mesh 的胖瘦会产生奇怪的结果
		// SplineMesh->SetRelativeScale3D(Scale);
		SplineMesh->SetStartScale(Fat, false);
		SplineMesh->SetEndScale(Fat, false);
		SplineMesh->UpdateMesh();
	}
	for (int32 i = SplineMeshComponents.Num() - 1; i >= DesiredMeshNumber; --i)
	{
		SplineMeshComponents[i]->DestroyComponent();
	}
}

void AMagnetSpline::DumpSplinePosition()
{
	static int cnt = 0;
	auto NumPoints = Spline->SplineCurves.Position.Points.Num();
	for (int32 i = 0; i < NumPoints; ++i)
	{
		auto Loc = Spline->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::World);
		UE_LOG(LogTemp, Log, TEXT("turn %d, Spline point %d: %s"), cnt, i, *Loc.ToString());
	}
	++cnt;
}