// Fill out your copyright notice in the Description page of Project Settings.


#include "Actor/AOCredit.h"
#include "Components/AOInteractComponent.h"
#include "Framework/AOPlayerController.h"

#define LOCTEXT_NAMESPACE "Credit"

AAOCredit::AAOCredit()
{
	CreditMesh = CreateDefaultSubobject<UStaticMeshComponent>("CreditMesh");
	SetRootComponent(CreditMesh);

	CreditInteract = CreateDefaultSubobject<UAOInteractComponent>("CreditInteract");
	CreditInteract->InteractActionText = LOCTEXT("CreditText", "Credit");
	CreditInteract->InteractItemName = LOCTEXT("CreditName", "Cache");
	CreditInteract->SetupAttachment(GetRootComponent());

	CreditAmount = 25;
}

#undef LOCTEXT_NAMESPACE
