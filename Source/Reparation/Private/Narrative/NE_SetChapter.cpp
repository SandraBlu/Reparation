// Fill out your copyright notice in the Description page of Project Settings.


#include "Narrative/NE_SetChapter.h"

#include "Framework/RPlayerState.h"
#include "Kismet/GameplayStatics.h"

UNE_SetChapter::UNE_SetChapter(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
	bRefireOnLoad = false;   // chapter is restored from our save data
}

void UNE_SetChapter::ExecuteEvent_Implementation(APawn* Pawn, APlayerController* Controller, UNarrativeComponent* NarrativeComponent)
{
	const UObject* WorldContext = Pawn ? static_cast<const UObject*>(Pawn) : static_cast<const UObject*>(Controller);
	if (!WorldContext) return;

	if (ARPlayerState* PS = Cast<ARPlayerState>(UGameplayStatics::GetPlayerState(WorldContext, 0)))
	{
		PS->SetCurrentChapter(Chapter);
	}
}
