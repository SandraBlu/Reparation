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

void ARPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	UpdateMagicCircleLocation();
}

void ARPlayerController::ShowTargetingCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(TargetCircle))
	{
		TargetCircle = GetWorld()->SpawnActor<ATargetCircle>(TargetCircleClass);
		if (DecalMaterial)
		{
			TargetCircle->TargetDecal->SetMaterial(0, DecalMaterial);
		}
	}
	
}

void ARPlayerController::HideTargetingCircle()
{
	if (IsValid(TargetCircle))
	{
		TargetCircle->Destroy();
	}
	
}

void ARPlayerController::CursorTrace()
{
	//const ECollisionChannel TraceChannel = IsValid(TargetCircle) ? ECC_ExcludePlayers : ECC_Visibility;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;
}

void ARPlayerController::UpdateMagicCircleLocation()
{
	if (IsValid(TargetCircle))
	{
		TargetCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}
