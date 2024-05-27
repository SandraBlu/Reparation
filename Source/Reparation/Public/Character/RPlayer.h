// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/RCharacter.h"
#include "Interface/RCombatInterface.h"
#include "Interface/RPlayerInterface.h"
#include "RPlayer.generated.h"


class URInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayer : public ARCharacter, public IRPlayerInterface
{
	GENERATED_BODY()

public:

	ARPlayer();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	//UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	//class UNiagaraComponent* LevelUpFX;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* DrawAnim;

	//UFUNCTION(BlueprintCallable)
	//bool CanAttack();
	////Set Attack States/Called In Attack Anim Montage-Use PrimaryAttack to set ATtack

	//UFUNCTION(BlueprintCallable)
	//void AttackEnd();
	//
	//bool CanDisarm();
	//bool CanDraw();
	//UFUNCTION(BlueprintCallable)
	//void Disarm();
	//UFUNCTION(BlueprintCallable)
	//void Draw();
	//UFUNCTION(BlueprintCallable)
	//void FinishEquipping();

	//UFUNCTION(BlueprintCallable)
	//void SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled);

	//EEquipState EquipState = EEquipState::EES_Unequipped;

	//UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	//EActionState ActionState = EActionState::EAS_Idle;

	//combat/ Player Interface
	virtual int32 GetCharacterLevel_Implementation() override;
	//virtual void AddToXP_Implementation(int32 InXP) override;
	//virtual void LevelUp_Implementation() override;
	//virtual int32 GetXP_Implementation() const override;
	//virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	//virtual int32 GetAttributePointsReward_Implementation(int32 Level) const override;
	//virtual int32 GetAbilityPointsReward_Implementation(int32 Level) const override;
	//virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	//virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
	//virtual void AddToAbilityPoints_Implementation(int32 InAbilityPoints) override;
	//virtual int32 GetAttributePoints_Implementation() const override;
	//virtual int32 GetAbilityPoints_Implementation() const override;

protected:

		virtual void InitAbilityActorInfo() override;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
		class USpringArmComponent* CameraBoom;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
		class UCameraComponent* FollowCam;

		virtual void BeginPlay() override;

		//Input 
		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputMappingContext* PlayerMappingContext;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputAction* MoveAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputAction* LookAction;

		UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
		UInputAction* AttackAction;

		//Input Binding Functions

		//void DrawWeapon(const FInputActionValue& Value);
	

public:

		// Called to bind functionality to input
		virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
private:
    	
    	UPROPERTY(EditDefaultsOnly, Category = Input)
    	URInputConfig* InputConfig;
    
    	void AbilityInputTagPressed(FGameplayTag InputTag);
    	void AbilityInputTagReleased(FGameplayTag InputTag);
    	void AbilityInputTagHeld(FGameplayTag InputTag);
	
};
