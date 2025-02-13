// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RCompanion.h"

#include "AI/AIC_Companion.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"


ARCompanion::ARCompanion()
{
}

void ARCompanion::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AIC_Companion = Cast<AAIC_Companion>(NewController);
	AIC_Companion->GetBlackboardComponent()->InitializeBlackboard(*BT_Companion->BlackboardAsset);
	AIC_Companion->RunBehaviorTree(BT_Companion);
	AIC_Companion->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
}
