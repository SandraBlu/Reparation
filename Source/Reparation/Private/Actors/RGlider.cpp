// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RGlider.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Components/ReactionComponent.h"
#include "Locomotion/RCharacterMovementComponent.h"
#include "Locomotion/RLocomotionComponent.h"

ARGlider::ARGlider()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// Queryable but not solid while it sits in the world: the interact check
	// traces on Visibility, so the wing has to answer that, but it should not
	// block anyone walking past it. Collision goes off entirely once worn.
	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	Mesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void ARGlider::OnAcquired(ACharacter* NewOwner)
{
	if (!NewOwner)
	{
		UE_LOG(LogTemp, Error, TEXT("%hs: [%s] acquired with no owning character."),
			__FUNCTION__, *GetNameSafe(this));
		return;
	}

	CarriedBy = NewOwner;
	SetOwner(NewOwner);

	AttachToComponent(
		NewOwner->GetMesh(),
		FAttachmentTransformRules::SnapToTargetNotIncludingScale,
		AttachSocket);

	// This wing's handling replaces whatever was there, which is what makes a
	// better glider actually better rather than merely a different mesh.
	if (URCharacterMovementComponent* Movement = Cast<URCharacterMovementComponent>(NewOwner->GetCharacterMovement()))
	{
		Movement->ApplyGlideSettings(GlideSettings);
	}

	if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(NewOwner))
	{
		if (UnlockTag.IsValid())
		{
			ASC->AddLooseGameplayTag(UnlockTag);
		}
	}

	// Worn, so no longer something in the world to walk up to. Both halves
	// matter: the reaction component would otherwise stay live on the wearer's
	// back, and a colliding actor strapped to a moving capsule fights both the
	// capsule and the world.
	if (UReactionComponent* Reaction = FindComponentByClass<UReactionComponent>())
	{
		Reaction->Deactivate();
	}

	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// Presentation follows locomotion rather than driving it, so deploy and stow
	// stay correct however a glide ends: released, landed, in water, or grabbing
	// a wall.
	Locomotion = NewOwner->FindComponentByClass<URLocomotionComponent>();
	if (Locomotion)
	{
		Locomotion->OnLocomotionStateChanged.AddDynamic(this, &ARGlider::HandleLocomotionStateChanged);

		// Bound after the fact, so catch up rather than waiting for a change.
		if (Locomotion->GetLocomotionState() == ERLocomotionState::Glide)
		{
			SetDeployed(true);
			return;
		}
	}

	SetDeployed(false);
}

void ARGlider::OnLost()
{
	if (Locomotion)
	{
		Locomotion->OnLocomotionStateChanged.RemoveDynamic(this, &ARGlider::HandleLocomotionStateChanged);
		Locomotion = nullptr;
	}

	if (CarriedBy)
	{
		if (UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(CarriedBy))
		{
			if (UnlockTag.IsValid())
			{
				ASC->RemoveLooseGameplayTag(UnlockTag);
			}
		}
	}

	// Interactable again once it is off the character.
	if (UReactionComponent* Reaction = FindComponentByClass<UReactionComponent>())
	{
		Reaction->Activate();
	}

	Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

	SetDeployed(false);

	// Visible again once dropped. SetDeployed hides it, which is right on a
	// wearer carrying it in a pack and wrong for one lying on the ground.
	Mesh->SetHiddenInGame(false);

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CarriedBy = nullptr;
	SetOwner(nullptr);
}

void ARGlider::HandleLocomotionStateChanged(ERLocomotionState PreviousState, ERLocomotionState NewState)
{
	if (NewState == ERLocomotionState::Glide)
	{
		SetDeployed(true);
	}
	else if (PreviousState == ERLocomotionState::Glide)
	{
		SetDeployed(false);
	}
}

void ARGlider::SetDeployed(bool bDeployed)
{
	Mesh->SetHiddenInGame(!bDeployed);

	// Reapplied on every deploy, not only on acquisition: skydiving writes its own
	// numbers onto the same movement properties, so the wing has to reclaim them
	// when it opens.
	if (bDeployed && CarriedBy)
	{
		if (URCharacterMovementComponent* Movement = Cast<URCharacterMovementComponent>(CarriedBy->GetCharacterMovement()))
		{
			Movement->ApplyGlideSettings(GlideSettings);
		}
	}

	if (bDeployed)
	{
		OnDeployed();
	}
	else
	{
		OnStowed();
	}
}
