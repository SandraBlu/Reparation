#pragma once

#include "CoreMinimal.h"
#include "GAS/Ability/RDamageAbility.h"
#include "Actors/Projectiles/RThrowable.h"
#include "RThrowMaster.generated.h"

class UAnimMontage;

/** Shared throw lifecycle: montage release notify, or immediate throw without a compatible montage. */
UCLASS(Abstract)
class REPARATION_API URThrowMaster : public URDamageAbility
{
    GENERATED_BODY()
public:
    URThrowMaster();
    /** Authoring helper for Blueprint children of GA_ThrowMaster. Sets identity and variant defaults. */
    UFUNCTION(BlueprintCallable, Category="Throw") void ConfigureThrowKind(ERThrowableKind NewKind);
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr, const FGameplayTagContainer* Target=nullptr, FGameplayTagContainer* Relevant=nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, const FGameplayEventData* Event) override;
    virtual FString GetDescription(int32 Level) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation, bool bReplicateEndAbility, bool bWasCancelled) override;
    virtual FString GetNextLevelDescription(int32 Level) override { return GetDescription(Level); }
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw") TSubclassOf<ARThrowable> ThrowableClass;
    /** Star uses this alternative when bThrowKnife is selected in GA_Star. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw") TSubclassOf<ARThrowable> KnifeClass;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw") bool bThrowKnife = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw") TObjectPtr<UAnimMontage> ThrowMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw") FName HandSocket = TEXT("weapon_r");
    /** Hold the loaded skill button to preview the arc; release it to start the throw montage. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw|Aim") bool bAimBeforeThrow = false;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw|Aim") TSubclassOf<class ARThrowAimPreview> AimPreviewClass;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw", meta=(ClampMin="1")) float LaunchSpeed = 1400.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw", meta=(ClampMin="0")) float GravityScale = 1.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw", meta=(ClampMin="1")) float AimDistance = 3000.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw", meta=(ClampMin="1")) float EffectRadius = 350.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw", meta=(ClampMin="0.1")) float EffectDuration = 6.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw", meta=(ClampMin="0.1")) float EffectInterval = 1.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Throw", meta=(ClampMin="1")) float HearingRange = 2000.f;
    /** Shared sight/sound range from the distraction's impact point, in cm. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Throw|Distraction", meta=(ClampMin="0")) float DistractionRadius = 2000.f;
    /** Only this many nearest eligible enemies investigate each grenade. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Throw|Distraction", meta=(ClampMin="0")) int32 MaxDistractedEnemies = 1;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Throw|Distraction") bool bSoundDistraction = true;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Throw|Distraction") bool bSightDistraction = true;
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Throw|Distraction") bool bDistractInCombat = true;
protected:
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Throw") ERThrowableKind Kind = ERThrowableKind::Star;
    UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category="Throw") FGameplayTag SkillTag;
private:
    UPROPERTY(Transient) TObjectPtr<class ARThrowAimPreview> AimPreview;
    FTimerHandle AimTimer;
    FVector AimedOrigin = FVector::ZeroVector;
    FVector AimedVelocity = FVector::ZeroVector;
    bool bWaitingForAim = false;
    void CalculateTrajectory(FVector& Origin, FVector& Velocity) const;
    void UpdateAim();
    void BeginThrow();
    UFUNCTION() void ConfirmAim(float HeldTime);
    UPROPERTY(Transient) TObjectPtr<ARThrowable> HeldProjectile;
    UPROPERTY(Transient) TObjectPtr<class UREquipItem_Weapon> SourceItem;
    TWeakObjectPtr<ARWeapon> SourceWeapon;
    bool bReleased = false;
    UFUNCTION() void Release(FGameplayEventData Payload);
    UFUNCTION() void Finished();
    UFUNCTION() void Interrupted();
};

UCLASS()
class REPARATION_API URDistraction : public URThrowMaster
{
    GENERATED_BODY()
public: URDistraction();
};

UCLASS()
class REPARATION_API URStar : public URThrowMaster
{
    GENERATED_BODY()
public: URStar();
};

UCLASS()
class REPARATION_API URSmokeGrenade : public URThrowMaster
{
    GENERATED_BODY()
public: URSmokeGrenade();
};

UCLASS()
class REPARATION_API URMolotov : public URThrowMaster
{
    GENERATED_BODY()
public: URMolotov();
};
