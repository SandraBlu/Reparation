// Copyright Narrative Tools 2024. 

#include "InteractionComponent.h"
#include "InteractableComponent.h"
#include <GameFramework/PlayerController.h>
#include <GameFramework/Pawn.h>
#include <Engine/World.h>
#include <Engine/EngineTypes.h>
#include <Engine/HitResult.h>

#define LOCTEXT_NAMESPACE "InteractionComponent"

// Sets default values for this component's properties
UNarrativeInteractionComponent::UNarrativeInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;

	CurrentInteractable = nullptr;
	LastInteractionCheckTime = 0.f;
	InteractionCheckDistance = 1000.f;
	RemainingInteractTime = -999.f;

	bInteractHeld = false;
	SetAutoActivate(true);
	SetIsReplicatedByDefault(true);
}


void UNarrativeInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningController = Cast<APlayerController>(GetOwner());

	if (OwningController)
	{
		OwningPawn = OwningController->GetPawn();
	}
}

void UNarrativeInteractionComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Server wasn't able to get this
	if (!OwningPawn && OwningController)
	{
		OwningPawn = OwningController->GetPawn();
	}

	if (IsActive())
	{
		if (GetWorld()->TimeSince(LastInteractionCheckTime) > InteractionCheckFrequency)
		{
			PerformInteractionCheck(DeltaTime);
		}

		if (bInteractHeld && CurrentInteractable)
		{
			if (RemainingInteractTime > 0.f)
			{
				RemainingInteractTime -= DeltaTime;
			}

			if (RemainingInteractTime <= 0.f && RemainingInteractTime > -10.f)
			{
				const bool bInteracted = CurrentInteractable->Interact(OwningPawn, this);

				if (bInteracted)
				{
					OnInteracted.Broadcast(this, CurrentInteractable);
				}

				RemainingInteractTime = -999.f;
			}
		}
	}
}

void UNarrativeInteractionComponent::Deactivate()
{
	Super::Deactivate();

	CouldntFindInteractable();
}

void UNarrativeInteractionComponent::PerformInteractionCheck(float DeltaTime)
{
	if (OwningController && OwningPawn)
	{
		FVector EyesLoc;
		FRotator EyesRot;

		OwningController->GetPlayerViewPoint(EyesLoc, EyesRot);

		//Add camera dist from pawn as long camera arms shouldn't effect how far you can interact 
		FVector TraceStart = EyesLoc;
		FVector TraceEnd = (EyesRot.Vector() * (InteractionCheckDistance + FVector::Dist(OwningPawn->GetActorLocation(), EyesLoc))) + TraceStart;
		FHitResult TraceHit;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwningPawn);

		if (FMath::IsNearlyZero(InteractionCheckSphereRadius))
		{
			GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		}
		else
		{
			const FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractionCheckSphereRadius);

			GetWorld()->SweepSingleByChannel(TraceHit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, Sphere, QueryParams);

		}

		//Check if we hit an interactable object
		if (TraceHit.GetActor())
		{
			DrawDebugSphere(GetWorld(), TraceHit.ImpactPoint, 10.f, 32, FColor::Red, false, 0.0f);
			if (UNarrativeInteractableComponent* InteractableComponent = Cast<UNarrativeInteractableComponent>(TraceHit.GetActor()->GetComponentByClass(UNarrativeInteractableComponent::StaticClass())))
			{
				if (InteractableComponent->IsActive())
				{
					const float Distance = (OwningPawn->GetActorLocation() - TraceHit.ImpactPoint).Size();

					if (Distance <= InteractableComponent->InteractionDistance)
					{
						FoundInteractable(InteractableComponent);
						return;
					}
				}
			}
		}
	}

	CouldntFindInteractable();
}

void UNarrativeInteractionComponent::CouldntFindInteractable()
{
	//Tell the interactable we've stopped focusing on it, and clear the current interactable
	if (CurrentInteractable)
	{
		if (CurrentInteractable)
		{
			CurrentInteractable->EndFocus(OwningPawn, this);
			OnLostInteractable.Broadcast(CurrentInteractable);
		}

		EndInteract();
	}

	CurrentInteractable = nullptr;
}

void UNarrativeInteractionComponent::FoundInteractable(UNarrativeInteractableComponent* Interactable)
{
	if (Interactable != CurrentInteractable)
	{
		EndInteract();

		if (CurrentInteractable)
		{
			CurrentInteractable->EndFocus(OwningPawn, this);
		}

		CurrentInteractable = Interactable;
		CurrentInteractable->BeginFocus(OwningPawn, this);
		OnFoundInteractable.Broadcast(Interactable);
	}
}

void UNarrativeInteractionComponent::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

void UNarrativeInteractionComponent::ServerEndInteract_Implementation()
{
	EndInteract();
}

void UNarrativeInteractionComponent::BeginInteract()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerBeginInteract();
	}
	
	bInteractHeld = true;

	OnInteractPressed.Broadcast(this);
	
	FText ErrorMessage;
	if (CurrentInteractable && CurrentInteractable->CanInteract(OwningPawn, this, ErrorMessage))
	{
		CurrentInteractable->BeginInteract(OwningPawn, this);
		RemainingInteractTime = CurrentInteractable->InteractionTime;
	}
}

void UNarrativeInteractionComponent::EndInteract()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerEndInteract();
	}

	if (CurrentInteractable)
	{
		CurrentInteractable->EndInteract(OwningPawn, this);
		RemainingInteractTime = -999.f;
	}

	bInteractHeld = false;
	OnInteractReleased.Broadcast(this);
}

#undef LOCTEXT_NAMESPACE
