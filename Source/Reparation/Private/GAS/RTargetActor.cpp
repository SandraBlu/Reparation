// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/RTargetActor.h"

#include "Abilities/GameplayAbility.h"

ARTargetActor::ARTargetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	TraceRange = 1500.0f;
	bDestroyOnConfirmation = true;
	ShouldProduceTargetDataOnServer = true;
	ReticleActor = nullptr;
}

bool ARTargetActor::LineTraceFN(FHitResult& TraceHitResult) const
{
	FVector ViewPoint;
	FRotator ViewRotation;
	PrimaryPC->GetPlayerViewPoint(ViewPoint, ViewRotation);
	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	
	if (APawn* MasterPawn = PrimaryPC->GetPawn())
	{
		QueryParams.AddIgnoredActor(MasterPawn->GetUniqueID());
	}

	bool HitResult = GetWorld()->LineTraceSingleByChannel(TraceHitResult, ViewPoint, ViewPoint + ViewRotation.Vector() * TraceRange, ECC_Visibility, QueryParams);

	return HitResult;
}

void ARTargetActor::StartTargeting(UGameplayAbility* Ability)
{
	OwningAbility = Ability;
	PrimaryPC = Cast<APlayerController>(Ability->GetOwningActorFromActorInfo()->GetInstigatorController());
	ReticleActor = SpawnReticleActor(GetActorLocation(), GetActorRotation());
}

void ARTargetActor::ConfirmTargetingAndContinue()
{
	FHitResult Hit;
	bool HitResult = LineTraceFN(Hit);
	FGameplayAbilityTargetDataHandle TargetData = StartLocation.MakeTargetDataHandleFromHitResult(OwningAbility, Hit);
	if (TargetData != nullptr)
	{
		TargetDataReadyDelegate.Broadcast(TargetData);
	}
	else
	{
		TargetDataReadyDelegate.Broadcast(FGameplayAbilityTargetDataHandle());
	}
	DestroyReticleActors();
}

void ARTargetActor::CancelTargeting()
{
	Super::CancelTargeting();
	DestroyReticleActors();
}

AGameplayAbilityWorldReticle* ARTargetActor::SpawnReticleActor(FVector Location, FRotator Rotation)
{
	if (ReticleClass)
	{
		if (!ReticleActor)
		{
			if (AGameplayAbilityWorldReticle* SpawnedReticleActor = GetWorld()->SpawnActor<AGameplayAbilityWorldReticle>(ReticleClass, Location, Rotation))
			{
				SpawnedReticleActor->InitializeReticle(this, PrimaryPC, ReticleParams);
				//SpawnedReticleActor->SetReplicates(false);
				return SpawnedReticleActor;
			}
		}
		else
		{
			return ReticleActor;
		}
	}
	return nullptr;
}

void ARTargetActor::DestroyReticleActors()
{
	if (ReticleActor)
	{
		ReticleActor->Destroy();
	}
}