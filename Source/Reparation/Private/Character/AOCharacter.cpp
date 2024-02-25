// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AOCharacter.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"


AAOCharacter::AAOCharacter()
{
	ASC = CreateDefaultSubobject<UAOAbilitySystemComponent>("ASC");
	Attributes = CreateDefaultSubobject<UAOAttributeSet>("AttributeSet");
}

UAbilitySystemComponent* AAOCharacter::GetAbilitySystemComponent() const
{
	return ASC;
}

void AAOCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	if (ASC)
		ASC->InitAbilityActorInfo(this, this);

	InitializeAttributes();
	GrantAbilities();
}

void AAOCharacter::InitializeAttributes()
{
	if (ASC && GrantedEffect)
	{
		FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(GrantedEffect, 1, EffectContext);

		if (SpecHandle.IsValid())
			FActiveGameplayEffectHandle GEHandle = ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
}
void AAOCharacter::GrantAbilities()
{
	if (ASC)
		for (TSubclassOf<UGameplayAbility>& StartupAbility : GrantedAbilities)
			ASC->GiveAbility(FGameplayAbilitySpec(StartupAbility.GetDefaultObject(), 1, 0));
}