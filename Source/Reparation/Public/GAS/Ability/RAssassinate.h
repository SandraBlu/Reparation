#pragma once
#include "GAS/Ability/RDamageAbility.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "RAssassinate.generated.h"

class AREnemy;
class ARPlayer;

UENUM(BlueprintType)
enum class ERAssassinationMode : uint8 { Ground, HideSpot, Air };

USTRUCT(BlueprintType)
struct FRAssassinationMontage
{
    GENERATED_BODY()
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimMontage> Montage = nullptr;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TObjectPtr<UAnimMontage> VictimMontage = nullptr;
};

/** Silent, notify-timed execution with separate ground, cover and airborne eligibility. */
UCLASS()
class REPARATION_API URAssassinate : public URDamageAbility
{
    GENERATED_BODY()
public:
    URAssassinate();
    virtual bool CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayTagContainer* Source=nullptr,const FGameplayTagContainer* Target=nullptr,FGameplayTagContainer* Relevant=nullptr) const override;
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation,const FGameplayEventData* Event) override;
    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
        const FGameplayAbilityActivationInfo Activation,bool bReplicate,bool bCancelled) override;
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override { return GetDescription(Level); }

    UFUNCTION(BlueprintPure,Category="Assassination") AREnemy* FindAssassinationTarget() const;
    UFUNCTION(BlueprintPure,Category="Assassination") bool CanAssassinateTarget(AREnemy* Enemy,ERAssassinationMode& Mode) const;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Montages") TArray<FRAssassinationMontage> GroundMontages;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Montages") TArray<FRAssassinationMontage> HideSpotMontages;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Montages") TArray<FRAssassinationMontage> AirMontages;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Rules",meta=(ClampMin="0")) float GroundRange=200.f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Rules",meta=(ClampMin="0")) float HideSpotRange=250.f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Rules",meta=(ClampMin="0")) float StrikeReach=250.f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Rules",meta=(ClampMin="0")) float QuietSpeed=180.f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Rules",meta=(ClampMin="0")) float QuietSeconds=.75f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Rules",meta=(ClampMin="0")) float ObserverRange=2500.f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Air",meta=(ClampMin="0.05",ClampMax="3")) float JumpPredictionTime=1.25f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Air",meta=(ClampMin="0")) float JumpPathRadius=45.f;
    UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Assassination|Air",meta=(ClampMin="0")) float MinimumAirHeight=80.f;
    UPROPERTY(BlueprintReadOnly,Transient,Category="Assassination") TObjectPtr<AREnemy> AssassinationTarget;
    UPROPERTY(BlueprintReadOnly,Transient,Category="Assassination") ERAssassinationMode ActiveMode=ERAssassinationMode::Ground;
private:
    friend class FAssassinateTest;
    bool EvaluateTarget(ARPlayer* Player,AREnemy* Enemy,ERAssassinationMode& Mode) const;
    AREnemy* FindTarget(ARPlayer* Player,ERAssassinationMode& Mode) const;
    bool IsUnseen(ARPlayer* Player) const;
    bool IsInJumpPath(ARPlayer* Player,const AREnemy* Enemy) const;
    bool HasClearReach(const ARPlayer* Player,const AREnemy* Enemy) const;
    const TArray<FRAssassinationMontage>& MontagesFor(ERAssassinationMode Mode) const;
    UFUNCTION() void Strike(FGameplayEventData Payload);
    UFUNCTION() void Finished();
    UFUNCTION() void Interrupted();
    bool bStrikeApplied=false;
    bool bTargetClaimed=false;
    bool bIgnoredMoveInput=false;
    EMovementMode SavedTargetMovement=MOVE_None;
    uint8 SavedTargetCustomMode=0;
    UPROPERTY(Transient) TObjectPtr<UAnimMontage> ActiveVictimMontage;
};
