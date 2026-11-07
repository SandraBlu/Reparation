#include "GAS/Ability/RJumpStrike.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "NativeGameplayTags.h"
#include "AI/REnemy.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Characters/RPlayer.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(JumpStrikeSkill,"ability.technique.jumpStrike");
UE_DEFINE_GAMEPLAY_TAG_STATIC(JumpStrikeCooldownTag,"cooldown.jumpStrike");

URJumpStrike::URJumpStrike()
{
    SetAssetTags(FGameplayTagContainer(JumpStrikeSkill));
    ActivationBlockedTags.AddTag(JumpStrikeCooldownTag);
    DamageMultiplier=2.f;
    PlayRate=1.f;
    TraceRadius=40.f;
    bResetHitsPerDamageWindow=false; // One damaging contact per target across the whole action.
}
FGameplayTag URJumpStrike::GetRequiredSkillTag() const { return JumpStrikeSkill; }
bool URJumpStrike::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,const FGameplayAbilityActorInfo* Info,
    const FGameplayTagContainer* Source,const FGameplayTagContainer* Target,FGameplayTagContainer* Relevant) const
{
    if (!Super::CanActivateAbility(Handle,Info,Source,Target,Relevant)) return false;
    const auto* Character=Info ? Cast<ACharacter>(Info->AvatarActor.Get()) : nullptr;
    return Character && Character->GetCharacterMovement() && Character->GetCharacterMovement()->IsMovingOnGround();
}
FString URJumpStrike::GetDescription(int32 Level)
{
    return FString::Printf(TEXT("<Title>Jump Strike</>\n\n<Default>Perform a randomly selected katana jump attack from the ground. Sword contact damages each target once.</>\n\n<Small>Damage multiplier: </><Damage>%.2fx</>\n<Small>Cooldown: </><Cooldown>%.1f s</>\n<Small>Stagger: </><Cooldown>%.2f s</>\n<Default>Control-immune enemies resist the reaction. Uses your selected melee element. Assign to an active input slot.</>"),DamageMultiplier,GetCooldown(Level),StaggerSeconds);
}

void URJumpStrike::ProcessMeleeHit(const FHitResult& Hit)
{
    auto* Player=Cast<ARPlayer>(GetAvatarActorFromActorInfo());
    auto* Enemy=Cast<AREnemy>(Hit.GetActor());
    if (!Player || !Player->HasAuthority() || Player->IsDead_Implementation() || !Enemy || Enemy->IsDead_Implementation()
        || !URAbilitySystemLibrary::IsNotFriend(Player,Enemy)) return;
    Super::ProcessMeleeHit(Hit);
    Enemy->ApplyKnockdown(StaggerSeconds,Player);
}
