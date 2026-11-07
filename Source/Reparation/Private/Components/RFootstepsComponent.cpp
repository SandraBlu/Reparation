// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/RFootstepsComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "Characters/RPlayer.h"
#include "Framework/RPhysicalMaterial.h"
#include "Kismet/GameplayStatics.h"
#include "RTypes/REnumTypes.h"
#include "Stealth/RStealthComponent.h"
#include "Perception/AISense_Hearing.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/World.h"

float URFootstepsComponent::GetNoiseHearingRange() const
{
	const auto* Player=Cast<ARPlayer>(GetOwner());
	return FMath::Max(0.f,NoiseHearingRange)*(Player && Player->Stealth ? Player->Stealth->GetMovementNoiseRangeMultiplier() : 1.f);
}


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
					if (GetWorld()->IsGameWorld() && Character->HasAuthority() && Character->GetCharacterMovement()->IsMovingOnGround()
						&& Character->GetVelocity().SizeSquared2D()>100.f)
					{
						const float Loudness=Character->bIsCrouched ? FMath::Clamp(CrouchedNoiseLoudness,0.f,1.f) : 1.f;
						if (Character->Stealth) Character->Stealth->RecordNoise(Loudness);
						const float Range=GetNoiseHearingRange();
						// Zero MaxRange means unlimited in Unreal, so skip a silenced event entirely.
						if (Range>0.f && Loudness>0.f)
							UAISense_Hearing::ReportNoiseEvent(this,HitResult.ImpactPoint,Loudness,Character,Range,TEXT("Footstep"));
					}
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
