// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RCharacter.h"
#include "RPlayer.generated.h"


class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayer : public ARCharacter
{
	GENERATED_BODY()

	public:

		ARPlayer();

	protected:

		UPROPERTY(VisibleAnywhere)
		class USpringArmComponent* CameraBoom;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class UCameraComponent* FollowCam;

		virtual void BeginPlay() override;
		//virtual void Tick(float DeltaTime) override;

		//Input 
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputMappingContext* PlayerMappingContext;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputAction* MoveAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputAction* LookAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputAction* AttackAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputAction* InterAction;

		//Input Binding Functions
		//void Move(const FInputActionValue& Value);
		//void Look(const FInputActionValue& Value);

		//void DrawWeapon(const FInputActionValue& Value);
		//void PrimaryAttack(const FInputActionValue& Value);

	public:

		// Called to bind functionality to input
		virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
};
