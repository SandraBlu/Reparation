// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "RCharacterBase.generated.h"

UCLASS()
class REPARATION_API ARCharacterBase : public ACharacter
{
	GENERATED_BODY()

public:
	
	ARCharacterBase();

protected:
	
	virtual void BeginPlay() override;
	

	
	

};
