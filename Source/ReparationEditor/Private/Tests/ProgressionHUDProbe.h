#pragma once
#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GAS/Ability/RMeleeAbility.h"
#include "Animation/AnimMontage.h"
#include "ProgressionHUDProbe.generated.h"

UCLASS()
class UProgressionHUDProbe : public UObject
{
    GENERATED_BODY()
public:
    int32 Level = 0;
    float XPProgress = -1.f;
    UFUNCTION() void ReceiveLevel(int32 Value) { Level = Value; }
    UFUNCTION() void ReceiveXP(float Value) { XPProgress = Value; }
};

UCLASS(NotBlueprintable, Transient)
class URComboProgressionTestAbility : public URMeleeAbility
{
    GENERATED_BODY()
public:
    URComboProgressionTestAbility() { NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly; }
    void Configure()
    {
        bUseDamageWindowForCombo = true;
        for (int32 Rank = 1; Rank <= 3; ++Rank)
        {
            FRMeleeAttack Attack;
            Attack.Montage = NewObject<UAnimMontage>(this);
            Attack.RequiredAbilityLevel = Rank;
            Attacks.Add(Attack);
        }
    }
    void ConfigureRandomLights()
    {
        Attacks.Reset(); AttackSelection=ERMeleeAttackSelection::RandomByCharacterLevel;
        for (int32 Level=1; Level<=4; ++Level)
        {
            FRMeleeAttack Attack; Attack.Montage=NewObject<UAnimMontage>(this);
            Attack.RequiredCharacterLevel=Level; Attacks.Add(Attack);
        }
    }
    int32 GetCurrentAttackIndexForTest() const { return CurrentAttackIndex; }
};
