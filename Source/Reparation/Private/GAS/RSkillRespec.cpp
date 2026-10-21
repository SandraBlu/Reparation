#include "GAS/RAbilitySystemComponent.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "Components/RInventoryComponent.h"
#include "RGameplayTags.h"
#include "GAS/Ability/RGameplayAbility.h"

bool URAbilitySystemComponent::IsTierRewardEarned()
{
    const UAbilityInfo* Data = GetSkillData();
    if (!Data || !Data->AutomaticTierReward.IsValid()) return false;
    int32 Completed = 0;
    for (ERSkillDiscipline Branch : {ERSkillDiscipline::Melee, ERSkillDiscipline::Ranged, ERSkillDiscipline::Stealth})
    {
        int32 Authored = 0, Learned = 0;
        TSet<FGameplayTag> Seen;
        for (const FRAbilityInfo& Entry : Data->AbilityInfo)
            if (Entry.Discipline == Branch && Entry.BranchTier == Data->RewardTier && Entry.AbilityTag.IsValid() && !Seen.Contains(Entry.AbilityTag))
            {
                Seen.Add(Entry.AbilityTag); ++Authored;
                if (GetLearnedSkillRank(Entry.AbilityTag) > 0) ++Learned;
            }
        // Missing planned abilities never shrink a full tier into an easier reward.
        if (Authored >= FMath::Max(1,Data->RewardSkillsPerBranch) && Learned == Authored) ++Completed;
    }
    return Completed >= (Data->bRewardRequiresAllBranches ? 3 : 1);
}

void URAbilitySystemComponent::RefreshTierReward()
{
    const UAbilityInfo* Data = GetSkillData();
    if (!IsOwnerActorAuthoritative() || !Cast<ARPlayer>(GetAvatarActor()) || !Data || !Data->AutomaticTierReward.IsValid()) return;
    const FRAbilityInfo Reward = Data->FindAbilityInfoForTag(Data->AutomaticTierReward);
    if (!Reward.Ability) return;
    FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(Reward.AbilityTag);
    const auto& T = FRGameplayTags::Get();
    if (IsTierRewardEarned())
    {
        if (!Spec)
        {
            FGameplayAbilitySpec NewSpec(Reward.Ability,1);
            NewSpec.GetDynamicSpecSourceTags().AddTag(T.ability_status_equipped);
            if (auto* Ability = Cast<URGameplayAbility>(NewSpec.Ability); Ability && Ability->StartupInputTag.IsValid())
                NewSpec.GetDynamicSpecSourceTags().AddTag(Ability->StartupInputTag);
            GiveAbility(NewSpec);
            ClientUpdateAbilityStatus(Reward.AbilityTag,T.ability_status_equipped,1);
        }
    }
    else if (Spec)
    {
        const FGameplayAbilitySpecHandle Handle = Spec->Handle;
        ServerUnequipAbility_Implementation(Reward.AbilityTag);
        CancelAbilityHandle(Handle);
        ClearAbility(Handle);
        ClientUpdateAbilityStatus(Reward.AbilityTag,T.ability_status_locked,1);
    }
}

bool URAbilitySystemComponent::IsBranchTierUnlocked(ERSkillDiscipline Branch, int32 Tier, int32 Required)
{
    if (Tier <= 1) return true;
    const UAbilityInfo* Data = GetSkillData();
    if (!Data || Tier > 5) return false;
    int32 Learned = 0;
    for (const FRAbilityInfo& Entry : Data->AbilityInfo)
        if (Entry.Discipline == Branch && Entry.BranchTier == Tier - 1 && GetLearnedSkillRank(Entry.AbilityTag) > 0) ++Learned;
    return Learned >= FMath::Max(1, Required);
}

int32 URAbilitySystemComponent::GetBranchRefund(ERSkillDiscipline Branch) const
{
    const UAbilityInfo* Data = GetSkillData();
    int64 Total = 0;
    if (Data) for (const FRSkillPurchaseRecord& R : SkillPurchases)
        if (Data->FindAbilityInfoForTag(R.Skill).Discipline == Branch) Total += R.Points;
    return static_cast<int32>(FMath::Clamp<int64>(Total, 0, MAX_int32));
}

FText URAbilitySystemComponent::GetBranchRespecIssue(ERSkillDiscipline Branch)
{
    if (Branch != ERSkillDiscipline::Melee && Branch != ERSkillDiscipline::Ranged && Branch != ERSkillDiscipline::Stealth)
        return FText::FromString(TEXT("Select a skill in Melee, Ranged or Stealth to reset that branch."));
    const UAbilityInfo* Data = GetSkillData();
    const ARPlayer* Player = Cast<ARPlayer>(GetAvatarActor());
    const ARPlayerState* PS = Player ? Player->GetPlayerState<ARPlayerState>() : nullptr;
    if (!Data || !PS) return FText::FromString(TEXT("Player progression is unavailable."));
    const int32 Refund = GetBranchRefund(Branch);
    if (!Refund) return FText::FromString(TEXT("No purchased ranks in this branch to refund."));
    // Avoid cancelling a non-cancellable combat action or mutating an ability during its execution.
    for (const FGameplayAbilitySpec& Spec : GetActivatableAbilities())
        if (Data->FindAbilityInfoForTag(GetAbilityTagFromSpec(Spec)).Discipline == Branch && Spec.IsActive() && !IsPassiveAbility(Spec))
            return FText::FromString(TEXT("Finish the current skill before resetting this branch."));
    const int32 Cost = FMath::Max(1, Data->RespecCost);
    const int64 NetPoints = static_cast<int64>(PS->GetAbilityPts()) + Refund - (Data->RespecPayment == ERRespecPayment::SkillPoints ? Cost : 0);
    if (NetPoints < 0) return FText::FromString(TEXT("Not enough skill points to pay the respec fee."));
    if (NetPoints > MAX_int32) return FText::FromString(TEXT("Spend some points before claiming this refund."));
    if (Data->RespecPayment == ERRespecPayment::Gold && (!PS->PlayerInventory || PS->PlayerInventory->GetCurrency() < Cost))
        return FText::FromString(FString::Printf(TEXT("Requires %d gold."), Cost));
    if (Data->RespecPayment == ERRespecPayment::QuestItem && (Data->RespecItem.IsNull() || !PS->PlayerInventory || !PS->PlayerInventory->HasItem(Data->RespecItem, 1)))
        return FText::FromString(TEXT("Requires one configured quest respec item."));
    return FText::GetEmpty();
}

void URAbilitySystemComponent::ServerRespecBranch_Implementation(ERSkillDiscipline Branch)
{
    if (!GetBranchRespecIssue(Branch).IsEmpty()) return;
    const UAbilityInfo* Data = GetSkillData();
    ARPlayerState* PS = CastChecked<ARPlayer>(GetAvatarActor())->GetPlayerState<ARPlayerState>();
    const int32 Refund = GetBranchRefund(Branch), Cost = FMath::Max(1, Data->RespecCost);
    if (Data->RespecPayment == ERRespecPayment::Gold) PS->PlayerInventory->AddCurrency(-Cost);
    else if (Data->RespecPayment == ERRespecPayment::QuestItem)
    {
        if (PS->PlayerInventory->ConsumeItem(PS->PlayerInventory->FindItemByClass(Data->RespecItem), 1) != 1) return;
    }
    const auto& T = FRGameplayTags::Get();
    TArray<FRSkillPurchaseRecord> Reset;
    for (const FRSkillPurchaseRecord& R : SkillPurchases)
        if (Data->FindAbilityInfoForTag(R.Skill).Discipline == Branch) Reset.Add(R);
    // Update the ledger before callbacks so a repeated request cannot obtain another refund.
    SkillPurchases.RemoveAll([&](const FRSkillPurchaseRecord& R) { return Data->FindAbilityInfoForTag(R.Skill).Discipline == Branch; });
    for (const FRSkillPurchaseRecord& R : Reset)
    {
        CoreSkillPointsSpent = FMath::Max(0, CoreSkillPointsSpent - R.CorePoints);
        if (R.BaselineRank == 0) ServerUnequipAbility_Implementation(R.Skill);
        if (FGameplayAbilitySpec* Spec = GetSpecFromAbilityTag(R.Skill))
        {
            Spec->Level = FMath::Max(1, R.BaselineRank);
            if (R.BaselineRank == 0)
            {
                CancelAbilityHandle(Spec->Handle);
                Spec->GetDynamicSpecSourceTags().RemoveTag(T.ability_status_unlocked);
                Spec->GetDynamicSpecSourceTags().RemoveTag(T.ability_status_equipped);
                Spec->GetDynamicSpecSourceTags().AddTag(T.ability_status_available);
            }
            MarkAbilitySpecDirty(*Spec);
            ClientUpdateAbilityStatus(R.Skill, GetStatusFromSpec(*Spec), Spec->Level);
        }
    }
    PS->AddToAbilityPts(Refund - (Data->RespecPayment == ERRespecPayment::SkillPoints ? Cost : 0));
    RefreshTierReward();
    // Already learned shared knowledge and quest completion remain; new purchases recheck reduced investment.
}
