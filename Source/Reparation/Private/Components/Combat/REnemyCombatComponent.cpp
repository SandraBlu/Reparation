// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Combat/REnemyCombatComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "RDebugHelper.h"
#include "RGameplayTags.h"
#include "Framework/RAbilitySystemLibrary.h"

void UREnemyCombatComponent::OnHitTargetActor(AActor* HitActor)
{
	
	if (HitActor)
	{
		Debug::Print(GetOwningPawn()->GetActorNameOrLabel() + TEXT(" is hitting ") + HitActor->GetActorNameOrLabel());
	}
	if (OverlappedActors.Contains(HitActor))
	{
		return;
	}

	OverlappedActors.AddUnique(HitActor);
	
	bool bIsValidBlock = false;
	const FRGameplayTags GameplayTags = FRGameplayTags::Get();
	
	const bool bIsPlayerBlocking = URAbilitySystemLibrary::NativeDoesActorHaveTag(HitActor, GameplayTags.status_blocking);
	const bool bIsMyAttackUnblockable = false;

	if (bIsPlayerBlocking && !bIsMyAttackUnblockable)
	{
		bIsValidBlock = URAbilitySystemLibrary::IsValidBlock(GetOwningPawn(),HitActor);
	}

	FGameplayEventData EventData;
	EventData.Instigator = GetOwningPawn();
	EventData.Target = HitActor;

	if (bIsValidBlock)
	{
		//TODO::Handle successful block
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			HitActor,
			GameplayTags.status_blocking,
			EventData
		);
	}
	else
	{
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			GetOwningPawn(),
			GameplayTags.Event_HitMelee,
			EventData
		);
	}
}
