// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RPlayerController.h"

#include "Actors/TargetCircle.h"
#include "Components/DecalComponent.h"
#include "GameFramework/Character.h"
#include "UI/GAS/DamageTextComponent.h"

void ARPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bDodgedHit, bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent();
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bDodgedHit, bCriticalHit);
	}
}

ARPlayerController::ARPlayerController()
{
	bReplicates = true;
}