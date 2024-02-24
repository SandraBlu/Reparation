// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/AN_Footsteps.h"
#include "Character/AOCharacter.h"
#include "Components/AOFootstepsComponent.h"

void UAN_Footsteps::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	check(MeshComp);

	AAOCharacter* Character = MeshComp ? Cast<AAOCharacter>(MeshComp->GetOwner()) : nullptr;
	/*if (Character)
	{
		if (UAOFootstepsComponent* FootstepsComp = Character->GetFootstepsComp())
		{
			FootstepsComp->HandleFootstep(Foot);
		}
	}*/
}
