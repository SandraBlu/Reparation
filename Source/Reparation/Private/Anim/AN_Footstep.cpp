// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/AN_Footstep.h"

#include "Characters/RPlayer.h"
#include "Components/RFootstepsComponent.h"
#include "RTypes/REnumTypes.h"

void UAN_Footstep::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	check(MeshComp);

	ARPlayer* Character = MeshComp ? Cast<ARPlayer>(MeshComp->GetOwner()) : nullptr;
	if (Character)
	{
		if (URFootstepsComponent* FootstepsComp = Character->GetFootstepsComp())
		{
			FootstepsComp->HandleFootstep(Foot);
		}
	}
}
