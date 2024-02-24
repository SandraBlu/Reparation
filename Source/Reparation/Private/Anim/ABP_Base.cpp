// Fill out your copyright notice in the Description page of Project Settings.


#include "Anim/ABP_Base.h"
#include "Character/AOPlayerBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"

UABP_Base::UABP_Base()
{

}

void UABP_Base::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();
    OwningCharacter = Cast<AAOPlayerBase>(TryGetPawnOwner());

    if (OwningCharacter)
    {
        OwnerMovementComp = OwningCharacter->GetCharacterMovement();
    }
}

void UABP_Base::NativeUpdateAnimation(float DeltaTime)
{
    Super::NativeUpdateAnimation(DeltaTime);

    if (OwningCharacter)
    {
        if (OwnerMovementComp)
        {
            MovementMode = OwnerMovementComp->MovementMode;
            //EquipState = OwningCharacter->GetEquipState();
        }
    }
}
