// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/RCharacterBase.h"
#include "RPlayer.generated.h"

class UInputMappingContext;
class UREquipmentComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayer : public ARCharacterBase
{
	GENERATED_BODY()

public:
	
	ARPlayer();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UREquipmentComponent* Gear;


protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* FollowCam;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* PlayerMappingContext;


public:
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
};
