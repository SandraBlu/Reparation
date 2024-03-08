// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "AOPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class UAOInputConfig;
class UAOAbilitySystemComponent;

//Event handler for re Spawning Character
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPawnChanged, APawn*, NewPawn);


UCLASS()
class REPARATION_API AAOPlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	AAOPlayerController();

protected:

	virtual void BeginPlay() override;

		/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class AAOPlayerBase* GetPawnOwner() const;

	UPROPERTY()
	class AAOPlayerBase* ControlledPawn;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputMappingContext* PlayerMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = Input)
	UInputAction* InterAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* CrouchAction;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* OpenMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* OpenMapAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DrawWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ThrowAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* PrimaryAttackAction;

	//Input Binding Functions
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	/*void CrouchStart(const FInputActionValue& Value);
	void CrouchStop(const FInputActionValue& Value);*/

	void StartInteract(const FInputActionValue& Value);
	void CompleteInteract(const FInputActionValue& Value);

	void DrawWeapon(const FInputActionValue& Value);
	void PrimaryAttack(const FInputActionValue& Value);
	void UseThrowable(const FInputActionValue& Value);

	//Called to bind functionality to input
	virtual void SetupInputComponent() override;

private:

	//Custom Input

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	UAOInputConfig* InputConfig;
	
	UPROPERTY()
	UAOAbilitySystemComponent* AOAbilityComp;

	UAOAbilitySystemComponent* GetASC();

};
