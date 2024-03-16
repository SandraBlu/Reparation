// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Tasks/TargetData.h"
#include "GameFramework/Character.h"

UTargetData* UTargetData::GetProjectileTargetData(UGameplayAbility* OwningAbility)
{
	UTargetData* MyObj = NewAbilityTask<UTargetData>(OwningAbility);
	return MyObj;
}

void UTargetData::Activate()
{
	const APawn* Instigator = Cast<APawn>("Pawn");
	FVector TraceStart = Instigator->GetPawnViewLocation();
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FRotator TraceDirection = PC->GetControlRotation();
	PC->GetPlayerViewPoint(TraceStart, TraceDirection);
}

