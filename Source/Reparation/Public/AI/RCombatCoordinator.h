#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GAS/Data/RCombatBalanceSettings.h"
#include "RCombatCoordinator.generated.h"

class AREnemy;
class UGameplayAbility;

/** Shared, server-side permission for simultaneous enemy swings; targeting stays in the controller. */
UCLASS()
class REPARATION_API URCombatCoordinator : public UWorldSubsystem
{
    GENERATED_BODY()
public:
    FRCombatDifficultyProfile GetProfile() const;
    bool CanStartAttack(AREnemy* Enemy, AActor* Target);
    bool AcquireAttack(AREnemy* Enemy, AActor* Target, UGameplayAbility* Ability);
    void ReleaseAttack(AREnemy* Enemy, UGameplayAbility* Ability);
    int32 GetAttackerCount(AActor* Target);
private:
    struct FReservation { TWeakObjectPtr<AActor> Target; TWeakObjectPtr<UGameplayAbility> Ability; };
    TMap<TWeakObjectPtr<AREnemy>, FReservation> Reservations;
    void PruneReservations();
};
