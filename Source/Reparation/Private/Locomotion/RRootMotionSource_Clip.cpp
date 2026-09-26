// Fill out your copyright notice in the Description page of Project Settings.


#include "Locomotion/RRootMotionSource_Clip.h"

#include "Animation/AnimSequenceBase.h"

FRRootMotionSource_Clip::FRRootMotionSource_Clip()
{
	// Replaces the velocity rather than adding to it, so input cannot steer a
	// roll halfway through. Vertical is left to gravity.
	AccumulateMode = ERootMotionAccumulateMode::Override;
	Settings.SetFlag(ERootMotionSourceSettingsFlags::IgnoreZAccumulate);
}

FRootMotionSource* FRRootMotionSource_Clip::Clone() const
{
	return new FRRootMotionSource_Clip(*this);
}

bool FRRootMotionSource_Clip::Matches(const FRootMotionSource* Other) const
{
	if (!FRootMotionSource::Matches(Other))
	{
		return false;
	}

	// Matches already compared the struct types.
	return Clip == static_cast<const FRRootMotionSource_Clip*>(Other)->Clip;
}

void FRRootMotionSource_Clip::PrepareRootMotion(float SimulationTime, float MovementTickTime,
	const ACharacter& Character, const UCharacterMovementComponent& MoveComponent)
{
	RootMotionParams.Clear();

	if (Clip && MovementTickTime > UE_SMALL_NUMBER)
	{
		const float Length = Clip->GetPlayLength();
		const float From = FMath::Clamp(GetTime(), 0.f, Length);
		const float To = FMath::Clamp(GetTime() + SimulationTime, 0.f, Length);

		const FVector RootFrom = Clip->ExtractRootTrackTransform(FAnimExtractContext(static_cast<double>(From)), nullptr).GetLocation();
		const FVector RootTo = Clip->ExtractRootTrackTransform(FAnimExtractContext(static_cast<double>(To)), nullptr).GetLocation();

		FVector Moved = MeshToWorld.TransformVector(RootTo - RootFrom);
		Moved.Z = 0.f;

		// A velocity, as the movement component expects from a source. Dividing by
		// the movement tick rather than the simulated time is what lets a client
		// catching up apply several ticks of motion in one, as the engine's own
		// sources do.
		RootMotionParams.Set(FTransform(Moved / MovementTickTime));
	}

	SetTime(GetTime() + SimulationTime);
}

UScriptStruct* FRRootMotionSource_Clip::GetScriptStruct() const
{
	return FRRootMotionSource_Clip::StaticStruct();
}

FString FRRootMotionSource_Clip::ToSimpleString() const
{
	return FString::Printf(TEXT("[ID:%u]FRRootMotionSource_Clip %s %s"), LocalID, *InstanceName.GetPlainNameString(), *GetNameSafe(Clip));
}

void FRRootMotionSource_Clip::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Clip);

	FRootMotionSource::AddReferencedObjects(Collector);
}
