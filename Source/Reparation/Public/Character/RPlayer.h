// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RCharacter.h"
#include "RPlayer.generated.h"


class URInputConfig;
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

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UNiagaraComponent* LevelUpFX;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* DrawAnim;

protected:

	virtual void InitAbilityActorInfo();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* FollowCam;

	virtual void BeginPlay() override;

	//Input 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)UInputMappingContext* PlayerMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;

public:

		// Called to bind functionality to input
		virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
    	
    	/*UPROPERTY(EditDefaultsOnly, Category = Input)
    	URInputConfig* InputConfig;
    
    	void AbilityInputTagPressed(FGameplayTag InputTag);
    	void AbilityInputTagReleased(FGameplayTag InputTag);
    	void AbilityInputTagHeld(FGameplayTag InputTag);*/
};
