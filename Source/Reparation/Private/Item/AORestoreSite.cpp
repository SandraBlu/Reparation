// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/AORestoreSite.h"
#include "Components/AOInteractComponent.h"
#include "NiagaraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "Attributes/AOAttributeSet.h"
#include "Character/AOPlayerBase.h"
#include "AbilitySystemBlueprintLibrary.h"

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

void AAORestoreSite::ApplyEffectToTarget(AActor* TargetActor, TSubclassOf<UGameplayEffect> GameplayEffectClass)
{
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	if (TargetASC == nullptr) return;

	FGameplayEffectContextHandle EffectContext = TargetASC->MakeEffectContext();
	EffectContext.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpec = TargetASC->MakeOutgoingSpec(GameplayEffectClass, ActorLevel, EffectContext);
	const FActiveGameplayEffectHandle ActiveEffectHandle = TargetASC->ApplyGameplayEffectSpecToSelf(*EffectSpec.Data.Get());
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
	
	ApplyEffectToTarget(Character, EffectClass);
	
}

