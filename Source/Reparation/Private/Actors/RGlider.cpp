// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/RGlider.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "Locomotion/RCharacterMovementComponent.h"
#include "Locomotion/RLocomotionComponent.h"

ARGlider::ARGlider()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	SetRootComponent(Mesh);

	// Worn, not solid. A colliding actor strapped to a moving capsule fights
	// both the capsule and the world.
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
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
			OnDeployed();
			return;
		}
	}

	OnStowed();
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

	OnStowed();

	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	CarriedBy = nullptr;
	SetOwner(nullptr);
}

void ARGlider::HandleLocomotionStateChanged(ERLocomotionState PreviousState, ERLocomotionState NewState)
{
	if (NewState == ERLocomotionState::Glide)
	{
		OnDeployed();
	}
	else if (PreviousState == ERLocomotionState::Glide)
	{
		OnStowed();
	}
}
