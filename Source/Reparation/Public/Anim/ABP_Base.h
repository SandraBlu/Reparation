// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ABP_Base.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UABP_Base : public UAnimInstance
{
	GENERATED_BODY()

public:

		UABP_Base();

	virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTime) override;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class AAOPlayerBase* OwningCharacter;

    UPROPERTY(BlueprintReadOnly, Category = "Components")
    class UCharacterMovementComponent* OwnerMovementComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    class UAnimInstance* ABP;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    TEnumAsByte<EMovementMode> MovementMode;

    UPROPERTY(BlueprintReadOnly)
    float DeltaTick;

    //Derives from Header: Character/Character States
   /* UPROPERTY(BlueprintReadOnly, Category = "Weapon")
    EEquipState EquipState;*/
};
