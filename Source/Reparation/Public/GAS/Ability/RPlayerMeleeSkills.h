#pragma once
#include "GAS/Ability/RMeleeAbility.h"
#include "GAS/Ability/RGameplayAbility.h"
#include "RPlayerMeleeSkills.generated.h"

/** Purchased guard technique; uses the existing block control, not an active loadout slot. */
UCLASS()
class REPARATION_API URParryTechnique : public URGameplayAbility
{
    GENERATED_BODY()
public:
    URParryTechnique();
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override { return GetDescription(Level); }
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr, const FGameplayTagContainer* Target=nullptr, FGameplayTagContainer* Relevant=nullptr) const override { return false; }
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parry", meta=(ClampMin="0.05",ClampMax="1")) float ParryWindow = .25f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parry", meta=(ClampMin="0")) float StaminaCost = 5.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parry", meta=(ClampMin="0",ClampMax="3")) float StaggerSeconds = .6f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parry", meta=(ClampMin="1")) float MaximumRange = 300.f;
    /** Random successful-deflection animations, editable directly in GA_Parry. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parry|Animation") TArray<TObjectPtr<UAnimMontage>> ParryMontages;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parry|Animation") TArray<TObjectPtr<UAnimMontage>> StaffParryMontages;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Parry|Animation", meta=(ClampMin="0.1")) float ParryPlayRate = 1.f;
};

/** Early-game sword guard. Shares the established block input/tag */
UCLASS()
class REPARATION_API URSwordBlock : public URGameplayAbility
{
    GENERATED_BODY()
public:
    URSwordBlock();
    virtual FString GetDescription(int32 Level) override { return TEXT("<Title>Sword Block</>\n\n<Default>Hold your block input to guard with an equipped sword. Learn Parry to deflect a well-timed melee hit, then use learned Counterstrike for a follow-up.</>"); }
    virtual FString GetNextLevelDescription(int32 Level) override { return GetDescription(Level); }
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr,const FGameplayTagContainer* Target=nullptr,FGameplayTagContainer* Relevant=nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation,bool bReplicate,bool bCancelled) override;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Sword Block|Animation") TObjectPtr<UAnimMontage> GuardMontage;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Sword Block|Animation") TObjectPtr<UAnimMontage> StaffGuardMontage;
private:
    UAnimMontage* GetEquippedGuardMontage() const;
    bool bStaffGuard = false;
    UPROPERTY(Transient) TObjectPtr<UAnimMontage> ActiveGuardMontage;
    void PlayGuardPresentation(UAnimMontage* Montage,float Rate=1.f);
    UFUNCTION() void ParrySucceeded(AActor* Attacker,bool bInterrupted);
    UFUNCTION() void PresentationEnded(UAnimMontage* Montage,bool bInterrupted);
    UPROPERTY(Transient) TObjectPtr<UAnimMontage> Presentation;
};

UCLASS()
class REPARATION_API URHeavyStrikeTechnique : public UGameplayAbility
{
    GENERATED_BODY()
public:
    URHeavyStrikeTechnique();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr, const FGameplayTagContainer* Target=nullptr, FGameplayTagContainer* Relevant=nullptr) const override { return false; }
};
UCLASS()
class REPARATION_API URChainAttackTechnique : public URHeavyStrikeTechnique
{
    GENERATED_BODY()
public: URChainAttackTechnique();
};

/** Shared player execution for Counterstrike and Kick.*/
UCLASS()
class REPARATION_API URPlayerMeleeFollowup : public URMeleeAbility
{
    GENERATED_BODY()
public:
    URPlayerMeleeFollowup();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr, const FGameplayTagContainer* Target=nullptr, FGameplayTagContainer* Relevant=nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Event) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, bool bReplicate, bool bCancelled) override;
    UFUNCTION(BlueprintPure, Category="Melee") bool CanHitFollowupTarget(AActor* Target) const;
protected:
    friend class FPlayerMeleeSkillsTest;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Attack Settings", meta=(ClampMin="1")) float Reach = 240.f;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Attack Settings", meta=(ClampMin="0")) float DamageMultiplier = 1.5f;
    UPROPERTY(EditDefaultsOnly, Category="Melee|Kick", meta=(ClampMin="0",ClampMax="30")) float StaggerSeconds = .65f;
    virtual void ProcessMeleeHit(const FHitResult& Hit) override;
    UFUNCTION() void OpenHitWindow(FGameplayEventData Payload);
    UFUNCTION() void CloseHitWindow(FGameplayEventData Payload);
    UFUNCTION() void Completed();
    UFUNCTION() void Interrupted();
    void DamageTarget(AActor* Target);
    TWeakObjectPtr<AActor> CounterTarget;
    bool bCounterHit = false;
};

UCLASS()
class REPARATION_API URPlayerCounterstrike : public URPlayerMeleeFollowup
{ GENERATED_BODY() public: URPlayerCounterstrike(); };

/** Standalone player kick*/
UCLASS(HideCategories=("Melee|Counterstrike"))
class REPARATION_API URPlayerKick : public URPlayerMeleeFollowup
{
    GENERATED_BODY()
public:
    URPlayerKick();
    virtual FRMeleeAttack GetCurrentAttack() const override;
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override { return GetDescription(Level); }
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Kick", meta=(ToolTip="Used when moving forward. Attacks entry zero supplies the standing kick."))
    FRMeleeAttack ForwardKick;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Melee|Kick", meta=(ClampMin="0", Units="cm/s"))
    float ForwardKickMinimumSpeed = 50.f;
private:
    friend class URPlayerMeleeFollowup;
    UPROPERTY(Transient) FRMeleeAttack SelectedKickAttack;
};
