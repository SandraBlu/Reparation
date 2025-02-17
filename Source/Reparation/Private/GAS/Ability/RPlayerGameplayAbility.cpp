// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RPlayerGameplayAbility.h"
#include "Components/Combat/PlayerCombatComp.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerController.h"

ARPlayerController* URPlayerGameplayAbility::GetPlayerControllerFromActorInfo()
{
	if (!CachedPlayerController.IsValid())
	{
		CachedPlayerController = Cast<ARPlayerController>(CurrentActorInfo->PlayerController);
	}
	return CachedPlayerController.IsValid()? CachedPlayerController.Get() : nullptr;
}