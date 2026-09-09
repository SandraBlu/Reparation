// Fill out your copyright notice in the Description page of Project Settings.


#include "GAS/Ability/RFlamethrower.h"

#include "Actors/RWeapon.h"
#include "GameFramework/Character.h"
#include "Interfaces/RPlayerInterface.h"
#include "Kismet/KismetSystemLibrary.h"

void URFlamethrower::GetOwnerInfo()
{
	if (CurrentActorInfo)
	{
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void URFlamethrower::FindTarget(FVector& TargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<URPlayerInterface>())
	{
		if (ARWeapon* Weapon  = IRPlayerInterface::Execute_GetCurrentWeapon(OwnerCharacter))
		{
			TArray<AActor*> ActorsToIgnore;
			ActorsToIgnore.Add(OwnerCharacter);
			FHitResult HitResult;
			const FVector SocketLocation = Weapon->GetWeaponMesh()->GetSocketLocation(FName("fire"));
			UKismetSystemLibrary::SphereTraceSingle(OwnerCharacter, SocketLocation, TargetLocation, 20.f, TraceTypeQuery1,
				false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResult, true);
			if (HitResult.bBlockingHit)
			{
				TargetLocation = HitResult.ImpactPoint;
				CurrentTarget = HitResult.GetActor();
			}
		}
	}
}
