// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InteractComponent.h"

#include "Components/ReactionComponent.h"

// Sets default values for this component's properties
UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
	PrimaryComponentTick.bAllowTickOnDedicatedServer = true;

	CurrentReactor = nullptr;
	LastInteractCheckTime = 0.f;
	InteractCheckDistance = 1000.f;
	RemainingInteractTime = -999.f;

	bInteractHeld = false;
	SetAutoActivate(true);
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();

	OwningController = Cast<APlayerController>(GetOwner());

	if (OwningController)
	{
		OwningPawn = OwningController->GetPawn();
	}
	
}


// Called every frame
void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//Server wasn't able to get this
	if (!OwningPawn && OwningController)
	{
		OwningPawn = OwningController->GetPawn();
	}

	if (IsActive())
	{
		if (GetWorld()->TimeSince(LastInteractCheckTime) > InteractCheckFrequency)
		{
			PerformInteractCheck(DeltaTime);
		}

		if (bInteractHeld && CurrentReactor)
		{
			if (RemainingInteractTime > 0.f)
			{
				RemainingInteractTime -= DeltaTime;
			}

			if (RemainingInteractTime <= 0.f && RemainingInteractTime > -10.f)
			{
				const bool bInteracted = CurrentReactor->Interact(OwningPawn, this);

				if (bInteracted)
				{
					OnInteracted.Broadcast(this, CurrentReactor);
				}

				RemainingInteractTime = -999.f;
			}
		}
	}
}

void UInteractComponent::Deactivate()
{
	Super::Deactivate();

	NoReactorFound();
}

void UInteractComponent::PerformInteractCheck(float DeltaTime)
{
	if (OwningController && OwningPawn)
	{
		FVector EyesLoc;
		FRotator EyesRot;

		OwningController->GetPlayerViewPoint(EyesLoc, EyesRot);

		//Add camera dist from pawn as long camera arms shouldn't effect how far you can interact 
		FVector TraceStart = EyesLoc;
		FVector TraceEnd = (EyesRot.Vector() * (InteractCheckDistance + FVector::Dist(OwningPawn->GetActorLocation(), EyesLoc))) + TraceStart;
		FHitResult TraceHit;

		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(OwningPawn);

		if (FMath::IsNearlyZero(InteractCheckSphereRadius))
		{
			GetWorld()->LineTraceSingleByChannel(TraceHit, TraceStart, TraceEnd, ECC_Visibility, QueryParams);
		}
		else
		{
			const FCollisionShape Sphere = FCollisionShape::MakeSphere(InteractCheckSphereRadius);

			GetWorld()->SweepSingleByChannel(TraceHit, TraceStart, TraceEnd, FQuat::Identity, ECC_Visibility, Sphere, QueryParams);

		}

		//Check if we hit an interactable object
		if (TraceHit.GetActor())
		{
			DrawDebugSphere(GetWorld(), TraceHit.ImpactPoint, 10.f, 32, FColor::Red, false, 0.0f);
			if (UReactionComponent* ReactionComponent = Cast<UReactionComponent>(TraceHit.GetActor()->GetComponentByClass(UReactionComponent::StaticClass())))
			{
				if (ReactionComponent->IsActive())
				{
					const float Distance = (OwningPawn->GetActorLocation() - TraceHit.ImpactPoint).Size();

					if (Distance <= ReactionComponent->InteractDistance)
					{
						FoundReactor(ReactionComponent);
						return;
					}
				}
			}
		}
	}

	NoReactorFound();
}

void UInteractComponent::NoReactorFound()
{
	//Tell the interactable we've stopped focusing on it, and clear the current interactable
	if (CurrentReactor)
	{
		if (CurrentReactor)
		{
			CurrentReactor->EndFocus(OwningPawn, this);
			OnLostReaction.Broadcast(CurrentReactor);
		}

		EndInteract();
	}

	CurrentReactor = nullptr;
}

void UInteractComponent::FoundReactor(UReactionComponent* Reactor)
{
	if (Reactor != CurrentReactor)
	{
		EndInteract();

		if (CurrentReactor)
		{
			CurrentReactor->EndFocus(OwningPawn, this);
		}

		CurrentReactor = Reactor;
		CurrentReactor->BeginFocus(OwningPawn, this);
		OnFoundReaction.Broadcast(Reactor);
	}
}

void UInteractComponent::ServerBeginInteract_Implementation()
{
	BeginInteract();
}

void UInteractComponent::ServerEndInteract_Implementation()
{
	EndInteract();
}

void UInteractComponent::BeginInteract()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerBeginInteract();
	}
	
	bInteractHeld = true;

	OnInteractPressed.Broadcast(this);
	
	FText ErrorMessage;
	if (CurrentReactor && CurrentReactor->CanInteract(OwningPawn, this, ErrorMessage))
	{
		CurrentReactor->BeginInteract(OwningPawn, this);
		RemainingInteractTime = CurrentReactor->InteractTime;
	}
}

void UInteractComponent::EndInteract()
{
	if (GetOwnerRole() < ROLE_Authority)
	{
		ServerEndInteract();
	}

	if (CurrentReactor)
	{
		CurrentReactor->EndInteract(OwningPawn, this);
		RemainingInteractTime = -999.f;
	}

	bInteractHeld = false;
	OnInteractReleased.Broadcast(this);
}

#undef LOCTEXT_NAMESPACE

