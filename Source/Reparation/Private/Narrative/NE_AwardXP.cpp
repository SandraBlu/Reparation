// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/NE_AwardXP.h"

#include "Framework/RAbilitySystemLibrary.h"

UNE_AwardXP::UNE_AwardXP(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bRefireOnLoad = false;   // XP state comes from our own save data, not from Narrative replaying events
}

void UNE_AwardXP::ExecuteEvent_Implementation(APawn* Pawn, APlayerController* Controller, UNarrativeComponent* NarrativeComponent)
{
	const UObject* WorldContext = Pawn ? static_cast<const UObject*>(Pawn) : static_cast<const UObject*>(Controller);
	if (!WorldContext) return;

	URAbilitySystemLibrary::AwardXPToPlayer(WorldContext, Source, RewardID);
}
