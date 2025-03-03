// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "InputActionValue.h"
#include "Characters/RCharacterBase.h"
#include "Interfaces/RPlayerInterface.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "RPlayer.generated.h"

class URAbilitySystemLibrary;
class URFootstepsComponent;
class ARMelee;
class ATargetCircle;
struct FInputActionValue;
class UInputAction;
class UNiagaraComponent;
class URInputConfig;
class UInputMappingContext;
class UREquipmentComponent;
class URAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayer : public ARCharacterBase , public IRPlayerInterface
{
	GENERATED_BODY()

public:
	
	ARPlayer();

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	UREquipmentComponent* Gear;
	
	//Interfaces
	
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag) override;
	virtual AActor* GetCurrentEquippedWeapon_Implementation(ARWeapon* InWeapon) override;
	virtual void Die(const FVector& DeathImpulse) override;
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	virtual int32 GetXP_Implementation() const override;
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	virtual int32 GetAttributePtsReward_Implementation(int32 Level) const override;
	virtual int32 GetAbilityPtsReward_Implementation(int32 Level) const override;
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	virtual void AddToAttributePts_Implementation(int32 InAttributePoints) override;
	virtual void AddToAbilityPts_Implementation(int32 InAbilityPoints) override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual int32 GetAbilityPoints_Implementation() const override;
	virtual ARWeapon* GetCurrentWeapon_Implementation() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UNiagaraComponent* LevelUpFX;
	
	UPROPERTY()
	ARMelee* MeleeWeapon;
	
	URFootstepsComponent* GetFootstepsComp() const;
	
	UFUNCTION(Blueprintpure)
	bool GetIsUsingBlock();
	
	UFUNCTION(BlueprintCallable)
	bool SetIsUsingBlock(bool block );
	
protected:
	
	virtual void BeginPlay() override;
	
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	AActor* CombatTarget;
	
	//Input
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* PlayerMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* SwitchTarget;
	
	UPROPERTY(BlueprintReadOnly)
	URFootstepsComponent* FootstepComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Input)
	bool bUsingBlock;
	
private:

	//Input
	void Input_SwitchTargetTriggered(const FInputActionValue& Value);
	void Input_SwitchTargetCompleted(const FInputActionValue& Value);

	FVector2D SwitchTargetDirection = FVector2D::ZeroVector	;
	
	virtual void InitAbilityActorInfo() override;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	URInputConfig* InputConfig;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);
	
	UPROPERTY()
	URAbilitySystemComponent* RAbilitySystemComponent;
	
	URAbilitySystemComponent* GetASC();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpVFX();

	
public:
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
};
