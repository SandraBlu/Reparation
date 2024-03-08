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
	/*FVector ViewObject = Ability->GetCurrentActorInfo()->PlayerController->GetPawn()->GetPawnViewLocation();

	FHitResult Hit;

	FGameplayAbilityTargetDataHandle DataHandle;
	FGameplayAbilityTargetData_SingleTargetHit* Data = new FGameplayAbilityTargetData_SingleTargetHit();
	Data->HitResult = Hit;
	DataHandle.Add(Data);

	if (ShouldBroadcastAbilityTaskDelegates())
	{
		ViewData.Broadcast(DataHandle);
	}*/
}

