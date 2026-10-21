#pragma once
#include "GAS/Ability/RGameplayAbility.h"
#include "RElementalMastery.generated.h"

/** Knowledge unlocked by the sci-fi teaching quest; does not occupy an attack slot. */
UCLASS()
class REPARATION_API URAdvancedProjectileMastery : public UGameplayAbility
{
	GENERATED_BODY()
public:
	URAdvancedProjectileMastery();
	virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
		FGameplayTagContainer* OptionalRelevantTags = nullptr) const override { return false; }
};

/** Purchased knowledge; applied by compatible attacks rather than activated as an attack. */
UCLASS(Abstract)
class REPARATION_API URElementalMastery : public URGameplayAbility
{
    GENERATED_BODY()
public:
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayTagContainer* SourceTags = nullptr, const FGameplayTagContainer* TargetTags = nullptr,
        FGameplayTagContainer* OptionalRelevantTags = nullptr) const override { return false; }
    static FGameplayTag GetSkillForDamage(FGameplayTag Damage);
};
UCLASS() class REPARATION_API URFireMastery : public URElementalMastery { GENERATED_BODY() public: URFireMastery(); };
UCLASS() class REPARATION_API URIceMastery : public URElementalMastery { GENERATED_BODY() public: URIceMastery(); };
UCLASS() class REPARATION_API URElectricMastery : public URElementalMastery { GENERATED_BODY() public: URElectricMastery(); };
UCLASS() class REPARATION_API URPoisonMastery : public URElementalMastery { GENERATED_BODY() public: URPoisonMastery(); };
UCLASS() class REPARATION_API URDarkMastery : public URElementalMastery { GENERATED_BODY() public: URDarkMastery(); };
