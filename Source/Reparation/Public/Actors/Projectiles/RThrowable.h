#pragma once

#include "CoreMinimal.h"
#include "Actors/RWeapon.h"
#include "RAbilityTypes.h"
#include "RThrowable.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class UNiagaraSystem;
class UNiagaraComponent;

UENUM(BlueprintType)
enum class ERThrowableKind : uint8 { Distraction, Star, Smoke, Molotov, Blast };

/** Local single-player throwable. Damage parameters survive the launching ability ending. */
UCLASS()
class REPARATION_API ARThrowable : public ARWeapon
{
    GENERATED_BODY()
public:
    ARThrowable();
    /** Inventory-spawned weapons remain inert until explicitly launched by a throw ability. */
    UFUNCTION(BlueprintCallable, Category="Throwable") void Launch(FVector InitialVelocity);
    UFUNCTION(BlueprintPure, Category="Throwable") bool IsInFlight() const { return bInFlight; }
    virtual void OnEquip() override;
    virtual void OnUnEquip() override;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<USphereComponent> Collision;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UStaticMeshComponent> Mesh;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly) TObjectPtr<UProjectileMovementComponent> Movement;
    /** Follows the throwable in flight and stops at the first impact. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects|Motion") TObjectPtr<UNiagaraSystem> MotionVFX;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Effects|Motion") TObjectPtr<UNiagaraComponent> MotionEffect;
    /** Separate Niagara system played when the throwable hits. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects|Impact") TObjectPtr<UNiagaraSystem> ImpactVFX;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Effects|Impact") TObjectPtr<USoundBase> ImpactSound;
    UPROPERTY(BlueprintReadOnly) FDamageEffectParams DamageParams;
    UPROPERTY(BlueprintReadOnly) ERThrowableKind Kind = ERThrowableKind::Star;
    UPROPERTY(BlueprintReadOnly) float Radius = 350.f;
    UPROPERTY(BlueprintReadOnly) float Duration = 6.f;
    UPROPERTY(BlueprintReadOnly) float PulseInterval = 1.f;
    UPROPERTY(BlueprintReadOnly) float NoiseRange = 2000.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Distraction", meta=(ClampMin="0")) float DistractionRadius = 2000.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Distraction", meta=(ClampMin="0")) int32 MaxDistractedEnemies = 1;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Distraction") bool bSoundDistraction = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Distraction") bool bSightDistraction = true;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Throwable|Distraction") bool bDistractInCombat = true;
protected:
    virtual void BeginPlay() override;
    UFUNCTION() void OnHit(UPrimitiveComponent* Component, AActor* Other, UPrimitiveComponent* OtherComponent, FVector Impulse, const FHitResult& Hit);
    void PlayImpact(FVector Location);
private:
    friend class FThrowableAbilitiesTest;
    bool bDetonated = false;
    bool bInFlight = false;
    FTimerHandle PulseTimer;
    bool bDistractionTargetsChosen = false;
    TArray<TWeakObjectPtr<class ARAIController>> DistractionTargets;
    void DistractNearbyEnemies();
    void Pulse();
    void DamageTarget(AActor* Target);
};
