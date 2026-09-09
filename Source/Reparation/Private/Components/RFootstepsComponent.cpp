// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RFootstepsComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Characters/RPlayer.h"
#include "Framework/RPhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "RTypes/REnumTypes.h"


URFootstepsComponent::URFootstepsComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URFootstepsComponent::HandleFootstep(EFoot Foot)
{
	if (ARPlayer* Character = Cast<ARPlayer>(GetOwner()))
	{
		if (USkeletalMeshComponent* Mesh = Character->GetMesh())
		{
			FHitResult HitResult;
			const FVector SocketLocation = Mesh->GetSocketLocation(Foot == EFoot::Left ? LeftFootSocket : RightFootSocket);
			const FVector Location = SocketLocation + FVector::UpVector * 20;

			FCollisionQueryParams QueryParams;
			QueryParams.bReturnPhysicalMaterial = true;
			QueryParams.AddIgnoredActor(Character);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, Location, Location + FVector::UpVector * -50.f, ECollisionChannel::ECC_WorldStatic, QueryParams))
			{
				if (HitResult.bBlockingHit)
				{
					if (HitResult.PhysMaterial.Get())
					{
						URPhysicalMaterial* PhysicsMat = Cast<URPhysicalMaterial>(HitResult.PhysMaterial.Get());

						if (PhysicsMat)
						{
							UGameplayStatics::PlaySoundAtLocation(this, PhysicsMat->FootstepSFX, Location, 1.f);
							UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), PhysicsMat->FootstepVFX, Location);
						}
					}
				}
			}
		}
	}
}
