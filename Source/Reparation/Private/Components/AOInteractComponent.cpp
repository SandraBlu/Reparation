// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AOInteractComponent.h"
#include "Character/AOPlayerBase.h"
#include "UI/AOInteractWidget.h"
#include "Framework/AOPlayerController.h"

UAOInteractComponent::UAOInteractComponent()
{
	SetComponentTickEnabled(false);

	InteractTime = 0.f;
	InteractDistance = 1000.f;
	InteractItemName = FText::FromString("Interactable Object");
	InteractActionText = FText::FromString("Interact");

	Space = EWidgetSpace::Screen;
	DrawSize = FIntPoint(400, 100);
	bDrawAtDesiredSize = true;

	SetActive(true);
	SetHiddenInGame(true);
}

void UAOInteractComponent::SetInteractNameText(const FText& NewNameText)
{
	InteractItemName = NewNameText;
	RefreshWidget();
}

void UAOInteractComponent::SetInteractActionText(const FText& NewActionText)
{
	InteractActionText = NewActionText;
	RefreshWidget();
}

void UAOInteractComponent::Deactivate()
{
	Super::Deactivate();

	for (int32 i = Interactors.Num() - 1; i >= 0; --i)
	{
		if (AAOPlayerBase* Interactor = Interactors[i])
		{
			EndFocus(Interactor);
			EndInteract(Interactor);
		}
	}
	Interactors.Empty();
}

bool UAOInteractComponent::CanInteract(class AAOPlayerBase* Character) const
{
	return IsActive() && GetOwner() != nullptr && Character != nullptr;
}

void UAOInteractComponent::RefreshWidget()
{
	if (!bHiddenInGame)
	{
		if (UAOInteractWidget* InteractWidget = Cast<UAOInteractWidget>(GetUserWidgetObject()))
		{
			InteractWidget->UpdateInteractWidget(this);
		}
	}
}

void UAOInteractComponent::BeginFocus(class AAOPlayerBase* Character)
{
	if (!IsActive() || !GetOwner() || !Character)
	{
		return;
	}
	OnBeginFocus.Broadcast(Character);
	SetHiddenInGame(false);
	RefreshWidget();
}

void UAOInteractComponent::EndFocus(class AAOPlayerBase* Character)
{
	OnEndFocus.Broadcast(Character);
	SetHiddenInGame(true);
}

void UAOInteractComponent::BeginInteract(class AAOPlayerBase* Character)
{
	if (CanInteract(Character))
	{
		Interactors.AddUnique(Character);
		OnBeginInteract.Broadcast(Character);
	}
}

void UAOInteractComponent::EndInteract(class AAOPlayerBase* Character)
{
	Interactors.RemoveSingle(Character);
	OnEndInteract.Broadcast(Character);
}

void UAOInteractComponent::Interact(class AAOPlayerBase* Character)
{
	if (CanInteract(Character))
	{
		OnInteract.Broadcast(Character);
	}
}

float UAOInteractComponent::GetInteractPercentage(class AAOPlayerBase* Character)
{
	if (Interactors.IsValidIndex(0))
	{
		if (AAOPlayerBase* Interactor = Interactors[0])
		{
			if (IsValid(Character))
			{
				/*AAOPlayerController* PC = Cast<AAOPlayerController>(Character->GetController());*/

				if (Interactor && Interactor->IsInteracting())
				{
					return 1.f - FMath::Abs(Interactor->GetRemainingInteractTime() / InteractTime);
				}
			}
		}
	}
	return 0.f;
}
