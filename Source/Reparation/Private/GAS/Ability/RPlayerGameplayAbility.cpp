// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RPlayerGameplayAbility.h"
#include "Components/Combat/PlayerCombatComp.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerController.h"

ARPlayerController* URPlayerGameplayAbility::GetPlayerControllerFromActorInfo()
{
	if (!CachedWarriorHeroController.IsValid())
	{
		CachedWarriorHeroController = Cast<ARPlayerController>(CurrentActorInfo->PlayerController);
	}
	return CachedWarriorHeroController.IsValid()? CachedWarriorHeroController.Get() : nullptr;
}

UPlayerCombatComp* URPlayerGameplayAbility::GetPlayerCombatComponentFromActorInfo()
{
	return GetPlayerFromActorInfo()->GetPlayerCombatComp();
}
