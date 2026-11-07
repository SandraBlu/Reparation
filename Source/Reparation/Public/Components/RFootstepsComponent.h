// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RFootstepsComponent.generated.h"


enum class EFoot : uint8;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPARATION_API URFootstepsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	URFootstepsComponent();

	void HandleFootstep(EFoot Foot);
	UFUNCTION(BlueprintPure, Category="Footsteps|AI Noise") float GetNoiseHearingRange() const;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Footsteps|AI Noise", meta=(ClampMin="0"))
	float NoiseHearingRange=1000.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Footsteps|AI Noise", meta=(ClampMin="0",ClampMax="1"))
	float CrouchedNoiseLoudness=.35f;

protected:
	
	UPROPERTY(EditDefaultsOnly)
	FName LeftFootSocket = TEXT("foot_l");

	UPROPERTY(EditDefaultsOnly)
	FName RightFootSocket = TEXT("foot_r");	

		
};
