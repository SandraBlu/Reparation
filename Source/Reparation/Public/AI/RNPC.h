// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/RCharacterBase.h"
#include "RNPC.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API ARNPC : public ARCharacterBase
{
	GENERATED_BODY()

public:
	
	ARNPC();


	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Combat")
	USkeletalMeshComponent* Weapon;
	
	
};
