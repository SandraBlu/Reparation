// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NarrativeEvent.h"
#include "RTypes/REnumTypes.h" 
#include "NE_AwardXP.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UNE_AwardXP : public UNarrativeEvent
{
	GENERATED_BODY()

public:
	UNE_AwardXP(const FObjectInitializer& ObjectInitializer);

	// What kind of content this is. Picks the XP row in the LevelUpInfo asset.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="XP")
	ERXPSource Source = ERXPSource::MinorSide;

	// Unique and permanent, e.g. Quest_MQ_03 or Companion_Mira_Loyalty_1. Blocks double payment.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="XP")
	FName RewardID;

	virtual void ExecuteEvent_Implementation(APawn* Pawn, APlayerController* Controller, UNarrativeComponent* NarrativeComponent) override;
	
};
