// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Ability/RMissileAbility.h"
#include "Actors/RProjectile.h"
#include "Interface/RCombatInterface.h"
//#include "Kismet/KismetSystemLibrary.h"

URMissileAbility::URMissileAbility()
{
}

void URMissileAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	const bool bIsServer = HasAuthority(&ActivationInfo);
	if (!bIsServer) return;

	IRCombatInterface* CombatInterface = Cast<IRCombatInterface>(GetAvatarActorFromActorInfo());
	if (CombatInterface)
	{
		const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
		

		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		
		ARProjectile* Projectile = GetWorld()->SpawnActorDeferred<ARProjectile>(ProjectileClass, SpawnTransform, GetOwningActorFromActorInfo(),
			Cast<APawn>(GetOwningActorFromActorInfo()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		Projectile->FinishSpawning(SpawnTransform);
	}


	
}
//UKismetSystemLibrary::PrintString(this, FString("ActivateAbility (c++)"), true, false, FLinearColor::Yellow, 3);