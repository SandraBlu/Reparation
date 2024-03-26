// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AOCharacter.h"
#include "Abilities/AOAbilitySystemComponent.h"
#include "AbilitySystemComponent.h"
#include "Weapon/AOWeapon.h"
#include "Components/CapsuleComponent.h"
#include "AOGameplayTags.h"
#include "Reparation/Reparation.h"
#include "Framework/AOPlayerController.h"
#include "Components/AOInventoryComponent.h"
#include "Components/AOFootstepsComponent.h"
#include "Item/AOItem.h"


AAOCharacter::AAOCharacter()
{
	// Enabled on mesh to react to incoming projectiles
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
}

UAbilitySystemComponent* AAOCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAOCharacter::GrantAbilities()
{
	UAOAbilitySystemComponent* AOASC = CastChecked<UAOAbilitySystemComponent>(AbilitySystemComponent);

	AOASC->AddGrantedAbilities(GrantedAbilities);
}

void AAOCharacter::Die()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	bDead = true;
}

UAOFootstepsComponent* AAOCharacter::GetFootstepsComp() const
{
	return FootstepsComp;
}

void AAOCharacter::SetLootSource(class UAOInventoryComponent* NewLootSource)
{
	//if item is destroyed; stop looting
	if (NewLootSource && NewLootSource->GetOwner())
	{
		NewLootSource->GetOwner()->OnDestroyed.AddUniqueDynamic(this, &AAOCharacter::OnLootSourceDestroyed);
	}
	LootSource = NewLootSource;
	ShowHideLootMenu();
	UE_LOG(LogTemp, Warning, TEXT("set  new loot source"));

	//for AI
	if (NewLootSource)
	{
		if (AAOCharacter* Character = Cast<AAOCharacter>(NewLootSource->GetOwner()))
		{
			Character->SetLifeSpan(120.f);
		}
	}
}

bool AAOCharacter::IsLooting() const
{
	return LootSource != nullptr;
}

void AAOCharacter::LootItem(class UAOItem* ItemToGive)
{
	if (PlayerInventory && LootSource && ItemToGive && LootSource->HasItem(ItemToGive->GetClass(), ItemToGive->GetQuantity()))
	{
		const FItemAddResult AddResult = PlayerInventory->TryAddItem(ItemToGive);
		PlayerInventory->OnInventoryUpdated.Broadcast();
		if (AddResult.ActualAmountGiven > 0)
		{
			LootSource->ConsumeItem(ItemToGive, AddResult.ActualAmountGiven);
		}
		else
		{
			if (AAOPlayerController* PC = Cast<AAOPlayerController>(GetController()))
			{
				PC->ShowNotification(AddResult.ErrorText);
			}
		}
	}
}

FVector AAOCharacter::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	const FAOGameplayTags& GameplayTags = FAOGameplayTags::Get();
	if (MontageTag.MatchesTagExact(GameplayTags.Montage_Attack_Weapon) && IsValid(EquippedWeapon))
	{
		return EquippedWeapon->GetWeaponMesh()->GetSocketLocation(EquippedWeapon->FiringSocket);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.Montage_Attack_LHand))
	{
		return GetMesh()->GetSocketLocation(LHand);
	}
	if (MontageTag.MatchesTagExact(GameplayTags.Montage_Attack_RHand))
	{
		return GetMesh()->GetSocketLocation(RHand);
	}
	return FVector();
}

UAnimMontage* AAOCharacter::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void AAOCharacter::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> EffectClass, float Level) const
{
	if (GetAbilitySystemComponent() && EffectClass)
	{
		FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponent()->MakeEffectContext();
		EffectContext.AddSourceObject(this);
		const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(EffectClass, Level, EffectContext);

		GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
	}
}

void AAOCharacter::InitializeAttributes() const
{
	ApplyEffectToSelf(PrimaryAttributes, 1.f);
	ApplyEffectToSelf(SecondaryAttributes, 1.f);
	ApplyEffectToSelf(BaseAttributes, 1.f);
}

void AAOCharacter::InitAbilityActorInfo()
{

}

void AAOCharacter::BeginLootingNPC(class AAOCharacter* Character)
{
	if (Character)
	{
		Character->SetLootSource(PlayerInventory);
	}
}

void AAOCharacter::OnLootSourceDestroyed(AActor* DestroyedActor)
{
	if (LootSource && DestroyedActor == LootSource->GetOwner())
	{
		SetLootSource(nullptr);
	}
}

void AAOCharacter::ShowHideLootMenu()
{
	if (AAOPlayerController* PC = Cast<AAOPlayerController>(GetController()))
	{
		if (PC)
		{
			if (LootSource)
			{
				UE_LOG(LogTemp, Warning, TEXT("On Rep!!!!!!!! new loot source"));
				PC->ShowLootMenu(LootSource);
			}
			else
			{
				PC->HideLootMenu();
			}
		}
	}
}

TArray<FTaggedMontage> AAOCharacter::GetAttackMontages_Implementation()
{
	return AttackMontages;
}

bool AAOCharacter::IsDead_Implementation() const
{
	return bDead;
}

AActor* AAOCharacter::GetAvatar_Implementation()
{
	return this;
}
