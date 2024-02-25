// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AORestoreSite.h"
#include "Components/AOInteractComponent.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"
#include "Character/AOPlayerBase.h"

// Sets default values
AAORestoreSite::AAORestoreSite()
{
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>("MeshComp");
	MeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	SetRootComponent(MeshComp);

	RestoreSiteVFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RestoreSite"));
	RestoreSiteVFX->SetupAttachment(GetRootComponent());

	InteractComp = CreateDefaultSubobject<UAOInteractComponent>("RestoreInteractComp");
	InteractComp->InteractTime = 5.f;
	InteractComp->InteractDistance = 750.f;
	InteractComp->InteractItemName = FText::FromString("RestoreSite");
	InteractComp->InteractActionText = FText::FromString("Heal");
	InteractComp->SetupAttachment(MeshComp);

	RespawnTime = 5.f;

}

void AAORestoreSite::IsActive()
{
	RootComponent->SetVisibility(bIsActive, true);
}

void AAORestoreSite::ShowRestoreSite()
{
	SetRestoreSiteState(true);
}

void AAORestoreSite::HideAndCooldownRestoreSite()
{
	SetRestoreSiteState(false);

	GetWorldTimerManager().SetTimer(TimerHandle_RespawnTimer, this, &AAORestoreSite::ShowRestoreSite, RespawnTime);
}

void AAORestoreSite::SetRestoreSiteState(bool bNewIsActive)
{
	SetActorEnableCollision(bNewIsActive);

	//Set Visibility on root and all children
	RootComponent->SetVisibility(bNewIsActive, true);
}

void AAORestoreSite::OnInteract(class AAOPlayerBase* Character)
{
	if (IAbilitySystemInterface* ASCInterface = Cast<IAbilitySystemInterface>(Character))
	{
		const UAOAttributeSet* AttributeSet  =  Cast<UAOAttributeSet>(ASCInterface->GetAbilitySystemComponent()->GetAttributeSet(UAOAttributeSet::StaticClass()));
		UAOAttributeSet* MutableAttrSet = const_cast<UAOAttributeSet*>(AttributeSet);
		MutableAttrSet->SetHealth(AttributeSet->GetHealth() + 25.f);
	}
}

