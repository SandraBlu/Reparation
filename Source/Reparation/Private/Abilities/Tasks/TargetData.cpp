// Fill out your copyright notice in the Description page of Project Settings.


#include "Abilities/Tasks/TargetData.h"
#include "GameFramework/Character.h"

UTargetData* UTargetData::CreateTargetData(UGameplayAbility* OwningAbility)
{
	UTargetData* MyObj = NewAbilityTask<UTargetData>(OwningAbility);
	return MyObj;
}

void UTargetData::Activate()
{
	APlayerController* PC = Ability->GetCurrentActorInfo()->PlayerController.Get();
	FHitResult MissileTarget;

}

