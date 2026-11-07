// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NarrativeEvent.h"
#include "NE_SetChapter.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UNE_SetChapter : public UNarrativeEvent
{
	GENERATED_BODY()

public:
	UNE_SetChapter(const FObjectInitializer& ObjectInitializer);

	// The chapter the player is entering (1 to 10).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="XP")
	int32 Chapter = 1;

	virtual void ExecuteEvent_Implementation(APawn* Pawn, APlayerController* Controller, UNarrativeComponent* NarrativeComponent) override;
	
};
