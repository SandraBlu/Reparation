// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ReactionComponent.h"

#define LOCTEXT_NAMESPACE "ReactionComponent"

// Sets default values for this component's properties
UReactionComponent::UReactionComponent()
{InteractTime = 0.f;
	InteractDistance = 200.f;
	ReactorNameText = LOCTEXT("InteractNameText", "Interact Object");
	ReactorActionText = LOCTEXT("InteractActionText", "Interact");
	
	auto FocusedOverlayFinder = ConstructorHelpers::FObjectFinder<UMaterialInterface>(TEXT("/Script/Engine.Material'/NarrativeInteraction/Materials/M_Pulse.M_Pulse'"));
	if (FocusedOverlayFinder.Succeeded())
	{
		FocusedOverlayMaterial = FocusedOverlayFinder.Object;
	}

	SetAutoActivate(true);
	SetComponentTickEnabled(false);
}

void UReactionComponent::Deactivate()
{
	Super::Deactivate();
}

void UReactionComponent::SetReactorNameText(const FText& NewNameText)
{
	ReactorNameText = NewNameText;
}

void UReactionComponent::SetReactorActionText(const FText& NewActionText)
{
	ReactorActionText = NewActionText;
}

void UReactionComponent::BeginFocus(class APawn* Interactor, class UInteractComponent* InteractComp)
{
	if (!IsActive() || !GetOwner() || !Interactor)
	{
		return;
	}

	OnBeginFocus.Broadcast(Interactor, InteractComp);

	if (GetNetMode() != NM_DedicatedServer)
	{
		TInlineComponentArray<UMeshComponent*> Meshes;
		GetOwner()->GetComponents(Meshes);

		for (auto& Mesh : Meshes)
		{
			if (Mesh)
			{
				Mesh->SetOverlayMaterial(FocusedOverlayMaterial);
			}
		}
	}
}

void UReactionComponent::EndFocus(class APawn* Interactor, class UInteractComponent* InteractComp)
{
	OnEndFocus.Broadcast(Interactor, InteractComp);

	if (GetNetMode() != NM_DedicatedServer)
	{
		TInlineComponentArray<UMeshComponent*> Meshes;
		GetOwner()->GetComponents(Meshes);

		for (auto& Mesh : Meshes)
		{
			if (Mesh)
			{
				Mesh->SetOverlayMaterial(nullptr);
			}
		}
	}
}

void UReactionComponent::BeginInteract(class APawn* Interactor, class UInteractComponent* InteractComp)
{
	OnBeginInteract(Interactor, InteractComp);
	OnBeginInteracted.Broadcast(Interactor, InteractComp);
}

void UReactionComponent::EndInteract(class APawn* Interactor, class UInteractComponent* InteractComp)
{
	OnEndInteract(Interactor, InteractComp);
	OnEndInteracted.Broadcast(Interactor, InteractComp);
}

bool UReactionComponent::Interact(class APawn* Interactor, class UInteractComponent* InteractComp)
{
	FText ErrorMessage;

	const bool bCanInteract = CanInteract(Interactor, InteractComp, ErrorMessage);

	if (bCanInteract)
	{
		OnInteract(Interactor, InteractComp);
		OnInteracted.Broadcast(Interactor, InteractComp);
	}

	return bCanInteract;
}

void UReactionComponent::OnInteract_Implementation(class APawn* Interactor, class UInteractComponent* InteractComp)
{
}

void UReactionComponent::OnBeginInteract_Implementation(class APawn* Interactor, class UInteractComponent* InteractComp)
{
}

void UReactionComponent::OnEndInteract_Implementation(class APawn* Interactor, class UInteractComponent* InteractComp)
{
}

bool UReactionComponent::CanInteract_Implementation(class APawn* Interactor, class UInteractComponent* InteractComp,
	FText& OutErrorText)
{
	return true;
}

#undef LOCTEXT_NAMESPACE
// Called when the game starts
