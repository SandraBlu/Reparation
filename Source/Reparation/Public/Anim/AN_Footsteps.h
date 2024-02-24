// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_Footsteps.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UAN_Footsteps : public UAnimNotify
{
	GENERATED_BODY()
	
	public:

	virtual void  Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	/*UPROPERTY(EditAnywhere)
	EFoot Foot;*/
};
