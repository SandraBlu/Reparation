#pragma once
#include "GAS/Ability/RDamageAbility.h"
#include "RCoordinationTestAbility.generated.h"

/** Holds an actual GAS activation open so coordination ownership can be tested. */
UCLASS(NotBlueprintable, Transient)
class URCoordinationTestAbility : public URDamageAbility
{
    GENERATED_BODY()
public:
    URCoordinationTestAbility()
    {
        InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
        NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::ServerOnly;
        SetAssetTags(FGameplayTagContainer(FGameplayTag::RequestGameplayTag(TEXT("ability.attack.melee"))));
    }
};
