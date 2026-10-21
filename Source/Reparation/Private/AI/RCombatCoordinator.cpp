#include "AI/RCombatCoordinator.h"
#include "AI/REnemy.h"
#include "AI/RCompanion.h"
#include "Characters/RPlayer.h"
#include "Framework/RGameMode.h"
#include "Framework/RPlayerState.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/World.h"
#include "Stealth/RStealthComponent.h"

FRCombatDifficultyProfile URCombatCoordinator::GetProfile() const
{
    if (const ARGameMode* GM = GetWorld()->GetAuthGameMode<ARGameMode>()) return GM->GetCombatDifficultyProfile();
    return GetDefault<URCombatBalanceSettings>()->Normal;
}

void URCombatCoordinator::PruneReservations()
{
    for (auto It = Reservations.CreateIterator(); It; ++It)
    {
        const AREnemy* Enemy = It.Key().Get();
        const ARCharacterBase* Target = Cast<ARCharacterBase>(It.Value().Target.Get());
        const UGameplayAbility* Ability = It.Value().Ability.Get();
        if (!Enemy || Enemy->IsDead_Implementation() || !It.Value().Target.IsValid() || !Ability || !Ability->IsActive() ||
            (Target && (Target->IsDead_Implementation() || Target->Stealth->IsConcealed()))) It.RemoveCurrent();
    }
}

int32 URCombatCoordinator::GetAttackerCount(AActor* Target)
{
    PruneReservations();
    int32 Count = 0;
    for (const auto& Pair : Reservations) if (Pair.Value.Target.Get() == Target) ++Count;
    return Count;
}

bool URCombatCoordinator::CanStartAttack(AREnemy* Enemy, AActor* Target)
{
    if (!IsValid(Enemy) || !Enemy->HasAuthority() || Enemy->IsDead_Implementation() || !IsValid(Target)) return false;
    if (const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(Target); CharacterBase &&
        (CharacterBase->IsDead_Implementation() || CharacterBase->Stealth->IsConcealed())) return false;
    PruneReservations();
    if (Reservations.Contains(Enemy)) return true;
    const FRCombatDifficultyProfile Profile = GetProfile();
    int32 Limit = FMath::Clamp(Profile.CompanionAttackers, 1, 6);
    if (const ARPlayer* Player = Cast<ARPlayer>(Target))
    {
        const ARPlayerState* PS = Player->GetPlayerState<ARPlayerState>();
        Limit = Profile.GetPlayerAttackerLimit(PS ? PS->GetPlayerLevel() : 1);
    }
    else if (!Target->IsA<ARCompanion>()) return true;
    return GetAttackerCount(Target) < Limit;
}

bool URCombatCoordinator::AcquireAttack(AREnemy* Enemy, AActor* Target, UGameplayAbility* Ability)
{
    if (!Ability || !CanStartAttack(Enemy, Target)) return false;
    if (const FReservation* Existing = Reservations.Find(Enemy)) return Existing->Ability.Get() == Ability && Existing->Target.Get() == Target;
    FReservation Entry; Entry.Target = Target; Entry.Ability = Ability;
    Reservations.Add(Enemy, Entry);
    return true;
}

void URCombatCoordinator::ReleaseAttack(AREnemy* Enemy, UGameplayAbility* Ability)
{
    if (const FReservation* Entry = Reservations.Find(Enemy); Entry && Entry->Ability.Get() == Ability) Reservations.Remove(Enemy);
}
