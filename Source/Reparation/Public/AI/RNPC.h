// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RCharacter.h"
#include "RNPC.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API ARNPC : public ARCharacter
{
	GENERATED_BODY()

public:

	ARNPC();

protected:

	virtual void BeginPlay();

	UPROPERTY(EditAnywhere, Category = "Combat")
	USkeletalMeshComponent* Weapon;
	
	
};
