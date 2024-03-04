// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "AONPC.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API AAONPC : public AAOCharacter
{
	GENERATED_BODY()

public:

	AAONPC();

	virtual void BeginPlay() override;
	
};
