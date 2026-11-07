// Fill out your copyright notice in the Description page of Project Settings.
#include "GAS/Ability/RMeleeAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Actors/RWeapon.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/Combat/PawnCombatComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Interfaces/RPlayerInterface.h"
#include "GAS/RAbilitySystemComponent.h"
#include "Characters/RPlayer.h"
#include "Framework/RPlayerState.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"
#include "HAL/IConsoleManager.h"

static TAutoConsoleVariable<int32> CVarMeleeDebug(TEXT("r.Combat.MeleeDebug"),1,
    TEXT("Log player melee bank selection and playback at ability end. Set to 0 to disable."));

TArray<int32> URMeleeAbility::GetAvailableAttackIndices() const
{
	const auto& MontageOptions = GetAttackMontages();
	TArray<int32> Result;
	if (bUseTechniquePurchases)
	{
		for (int32 Index=0; Index<MontageOptions.Num(); ++Index)
			if (IsValid(MontageOptions[Index].Montage)) Result.Add(Index);
		return Result;
	}
	if (AttackSelection == ERMeleeAttackSelection::RandomByCharacterLevel)
	{
		const auto* Player = Cast<ARPlayer>(GetAvatarActorFromActorInfo());
		const auto* PS = Player ? Player->GetPlayerState<ARPlayerState>() : nullptr;
		const int32 Level = PS ? PS->GetPlayerLevel() : 1;
		for (int32 Index=0; Index<MontageOptions.Num(); ++Index)
			if (IsValid(MontageOptions[Index].Montage) && Level >= FMath::Clamp(MontageOptions[Index].RequiredCharacterLevel,1,40)) Result.Add(Index);
	}
	else
	{
		for (int32 Index=0; Index<MontageOptions.Num(); ++Index)
		{
			const int32 Required = Index==0 ? 1 : FMath::Max(2,MontageOptions[Index].RequiredAbilityLevel);
			if (!IsValid(MontageOptions[Index].Montage) || GetDamageScalingLevel()<Required) break;
			Result.Add(Index);
		}
	}
	return Result;
}

URMeleeAbility::URMeleeAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

bool URMeleeAbility::UsesStaffMontages() const
{
    const ARWeapon* Weapon = GetMeleeWeapon();
    return !bTraceFromCharacterMesh && Weapon && Weapon->CombatType == ECombatType::ECT_Staff;
}

const TArray<FRMeleeAttack>& URMeleeAbility::GetAttackMontages() const
{
    if (bAttackMontagesLatched) return ActiveAttackMontages;
    if (HasPurchasedCombo()) return UsesStaffMontages() ? StaffComboAttacks : ComboAttacks;
    return UsesStaffMontages() ? StaffAttacks : Attacks;
}

bool URMeleeAbility::HasPurchasedCombo() const
{
    auto* ASC = Cast<URAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
    return bUseTechniquePurchases && ASC && ASC->GetLearnedSkillRank(FGameplayTag::RequestGameplayTag(TEXT("ability.melee.combo2"))) > 0;
}

bool URMeleeAbility::UsesMontageAlternatives() const
{
    return bUseTechniquePurchases || AttackSelection == ERMeleeAttackSelection::RandomByCharacterLevel;
}

UPawnCombatComponent* URMeleeAbility::GetCombatComponentFromActorInfo()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	return IsValid(Avatar) ? Avatar->FindComponentByClass<UPawnCombatComponent>() : nullptr;
}

ARWeapon* URMeleeAbility::GetMeleeWeapon() const
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!IsValid(Avatar) || !Avatar->Implements<URPlayerInterface>()) return nullptr;
	ARWeapon* Weapon = IRPlayerInterface::Execute_GetCurrentWeapon(Avatar);
	if (!IsValid(Weapon) || !Weapon->IsEquipped ||
		(Weapon->CombatType != ECombatType::ECT_LightSword && Weapon->CombatType != ECombatType::ECT_Staff))
	{
		return nullptr;
	}
	return Weapon;
}

bool URMeleeAbility::GetMeleeTraceLocations(FVector& Start, FVector& End) const
{
	Start = End = FVector::ZeroVector;
	if (bTraceFromCharacterMesh)
	{
		const auto* Character=Cast<ACharacter>(GetAvatarActorFromActorInfo());
		const auto* Body=Character ? Character->GetMesh() : nullptr;
		if (!Body || !Body->DoesSocketExist(TraceStartSocket) || !Body->DoesSocketExist(TraceEndSocket)) return false;
		Start=Body->GetSocketLocation(TraceStartSocket);
		End=Body->GetSocketLocation(TraceEndSocket);
		return true;
	}
	ARWeapon* Weapon = GetMeleeWeapon();
	USkeletalMeshComponent* Mesh = Weapon ? Weapon->GetWeaponMesh() : nullptr;
	if (!IsValid(Mesh) || !Mesh->DoesSocketExist(Weapon->MeleeStartSocket) ||
		!Mesh->DoesSocketExist(Weapon->MeleeEndSocket)) return false;
	Start = Mesh->GetSocketLocation(Weapon->MeleeStartSocket);
	End = Mesh->GetSocketLocation(Weapon->MeleeEndSocket);
	return true;
}

int32 URMeleeAbility::GetUnlockedAttackCount() const
{
	return GetAvailableAttackIndices().Num();
}

float URMeleeAbility::GetDamageScalingLevel() const
{
	int32 Rank = GetAbilityLevel();
	if (bUseTechniquePurchases && AttackSelection == ERMeleeAttackSelection::SequentialCombo)
		if (auto* ASC = Cast<URAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()))
		{
			if (ASC->GetLearnedSkillRank(FGameplayTag::RequestGameplayTag(TEXT("ability.technique.heavyStrike"))) > 0) Rank = FMath::Max(Rank,2);
			if (ASC->GetLearnedSkillRank(FGameplayTag::RequestGameplayTag(TEXT("ability.melee.combo2"))) > 0) Rank = FMath::Max(Rank,2);
		}
	return Rank;
}

bool URMeleeAbility::BeginMeleeAttack()
{
	if (!IsActive() || CurrentAttackIndex != INDEX_NONE) return false;
	ResetMeleeState();
	ARWeapon* Weapon = GetMeleeWeapon();
	ActiveAttackMontages = GetAttackMontages();
	bActiveFullCombo = HasPurchasedCombo();
	bAttackMontagesLatched = true;
	const TArray<int32> Available = GetAvailableAttackIndices();
	if (CVarMeleeDebug.GetValueOnGameThread() && Cast<ARPlayer>(GetAvatarActorFromActorInfo()) && Available.IsEmpty())
		UE_LOG(LogTemp,Warning,TEXT("MeleeDebug: %s has no usable montages (purchasedCombo=%d staff=%d techniquePurchases=%d)."),
			*GetName(),bActiveFullCombo,UsesStaffMontages(),bUseTechniquePurchases);
	if ((!bTraceFromCharacterMesh && !Weapon) || Available.IsEmpty())
    {
        // Blueprint activation may have no failure execution pin wired. Never leave
        // an attack active without a montage; subsequent input would only queue combos.
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return false;
    }
	FVector Start, End;
	if (bTraceFromCharacterMesh && !GetMeleeTraceLocations(Start,End))
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return false;
    }
	ActiveMeleeWeapon = Weapon;
	if (UsesMontageAlternatives())
	{
		// Predicted client/server activations use the same seed and select the same montage.
		const int32 PredictionSeed = CurrentActivationInfo.GetActivationPredictionKey().Current;
		FRandomStream Stream(PredictionSeed);
		CurrentAttackIndex = Available[PredictionSeed != 0 ? Stream.RandRange(0,Available.Num()-1) : FMath::RandRange(0,Available.Num()-1)];
	}
	else CurrentAttackIndex = Available[0];
	if (CVarMeleeDebug.GetValueOnGameThread() && Cast<ARPlayer>(GetAvatarActorFromActorInfo()))
	{
		auto* ASC=Cast<URAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());
		const int32 ComboRank=ASC ? ASC->GetLearnedSkillRank(FGameplayTag::RequestGameplayTag(TEXT("ability.melee.combo2"))) : 0;
		UE_LOG(LogTemp,Display,TEXT("MeleeDebug: BEGIN %s actor=%s comboRank=%d techniquePurchases=%d bank=%s staff=%d selected=%s index=%d options=%d."),
			*GetName(),*GetNameSafe(GetAvatarActorFromActorInfo()),ComboRank,bUseTechniquePurchases,
			bActiveFullCombo ? TEXT("Combo") : TEXT("Single/Other"),UsesStaffMontages(),*GetNameSafe(GetCurrentAttack().Montage.Get()),CurrentAttackIndex,Available.Num());
	}
	return true;
}

bool URMeleeAbility::QueueComboAttack()
{
	if (UsesMontageAlternatives()) return false;
	if (!IsActive() || CurrentAttackIndex == INDEX_NONE || !bComboWindowOpen ||
		!IsValid(ActiveMeleeWeapon) || GetMeleeWeapon() != ActiveMeleeWeapon ||
		CurrentAttackIndex + 1 >= GetUnlockedAttackCount()) return false;
	bComboInputQueued = true;
	return true;
}

void URMeleeAbility::SetComboWindowOpen(bool bOpen)
{
	bComboWindowOpen = bOpen && IsActive() && CurrentAttackIndex != INDEX_NONE;
}

bool URMeleeAbility::AdvanceCombo()
{
	if (UsesMontageAlternatives()) return false;
	if (!IsActive() || CurrentAttackIndex == INDEX_NONE || !bComboInputQueued ||
		!IsValid(ActiveMeleeWeapon) || GetMeleeWeapon() != ActiveMeleeWeapon ||
		CurrentAttackIndex + 1 >= GetUnlockedAttackCount()) return false;
	SetDamageWindowOpen(false);
	++CurrentAttackIndex;
	bComboInputQueued = bComboWindowOpen = bDamageWindowOpen = false;
	HitActors.Reset();
	return true;
}

FRMeleeAttack URMeleeAbility::GetCurrentAttack() const
{
	const auto& MontageOptions = GetAttackMontages();
	return MontageOptions.IsValidIndex(CurrentAttackIndex) ? MontageOptions[CurrentAttackIndex] : FRMeleeAttack();
}

void URMeleeAbility::SetDamageWindowOpen(bool bOpen)
{
	if (bUseDamageWindowForCombo) SetComboWindowOpen(bOpen);
	UWorld* World = GetWorld();
	const bool bCanOpen = bOpen && World && IsActive() && CurrentAttackIndex != INDEX_NONE &&
		IsMeleeTraceSourceValid();
	if (!bCanOpen)
	{
		bDamageWindowOpen = false;
		if (World) World->GetTimerManager().ClearTimer(MeleeTraceTimer);
		return;
	}
	// Repeated begin events must not restart the timer or reset the swing's hit history.
	if (bDamageWindowOpen && World->GetTimerManager().IsTimerActive(MeleeTraceTimer)) return;
	if ((bResetHitsPerDamageWindow || bActiveFullCombo) && !bDamageWindowOpen) HitActors.Reset();
	bDamageWindowOpen = true;
	const float Interval = FMath::IsFinite(TraceInterval) ? FMath::Max(0.001f, TraceInterval) : 0.01f;
	World->GetTimerManager().SetTimer(MeleeTraceTimer, this,
		&URMeleeAbility::TraceCurrentAttack, Interval, true);
}

void URMeleeAbility::TraceCurrentAttack()
{
	if (!IsActive() || !bDamageWindowOpen || !IsMeleeTraceSourceValid())
	{
		SetDamageWindowOpen(false);
		return;
	}
	TArray<FHitResult> Hits;
	if (bTraceFromCharacterMesh)
	{
		FVector Start,End;
		if (!GetMeleeTraceLocations(Start,End)) return;
		const TArray<TEnumAsByte<EObjectTypeQuery>> Objects={UEngineTypes::ConvertToObjectType(ECC_Pawn),UEngineTypes::ConvertToObjectType(ECC_WorldDynamic)};
		const TArray<AActor*> Ignore={GetAvatarActorFromActorInfo()};
		UKismetSystemLibrary::SphereTraceMultiForObjects(GetAvatarActorFromActorInfo(),Start,End,TraceRadius,Objects,false,Ignore,
			bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,Hits,false);
	}
	else Hits = URAbilitySystemLibrary::TraceMeleeWeapon(GetAvatarActorFromActorInfo(), TraceRadius, bDrawDebugTrace);
	for (const FHitResult& Hit : Hits)
	{
		if (RegisterMeleeHit(Hit.GetActor()))
		{
			ProcessMeleeHit(Hit);
		}
	}
}

bool URMeleeAbility::IsMeleeTraceSourceValid() const
{
    if (Cast<ARPlayer>(GetAvatarActorFromActorInfo()) && !GetMeleeWeapon()) return false;
	if (!bTraceFromCharacterMesh) return IsValid(ActiveMeleeWeapon) && GetMeleeWeapon()==ActiveMeleeWeapon;
	FVector Start,End;
	return GetMeleeTraceLocations(Start,End);
}

bool URMeleeAbility::ApplyMeleeHit(AActor* TargetActor)
{
	// Retain the explicit simple-damage helper for existing Blueprint callers.
	if (!DamageEffectClass || !GetAbilitySystemComponentFromActorInfo() || !RegisterMeleeHit(TargetActor))
	{
		return false;
	}
	CauseDamage(TargetActor);
	return true;
}

bool URMeleeAbility::RegisterMeleeHit(AActor* TargetActor)
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!IsActive() || !bDamageWindowOpen || !IsValid(Avatar) || !Avatar->HasAuthority() ||
		!IsMeleeTraceSourceValid() ||
		!IsValid(TargetActor) || TargetActor == Avatar || TargetActor == ActiveMeleeWeapon ||
		HitActors.Contains(TWeakObjectPtr<AActor>(TargetActor)) ||
		!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor)) return false;
	HitActors.Add(TWeakObjectPtr<AActor>(TargetActor));
	return true;
}

void URMeleeAbility::ResetMeleeState()
{
	SetDamageWindowOpen(false);
	CurrentAttackIndex = INDEX_NONE;
	bComboWindowOpen = bComboInputQueued = bDamageWindowOpen = false;
	ActiveMeleeWeapon = nullptr;
	bAttackMontagesLatched = false;
	bActiveFullCombo = false;
	ActiveAttackMontages.Reset();
	HitActors.Reset();
}

void URMeleeAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if (!IsEndAbilityValid(Handle, ActorInfo)) return;
	if (CVarMeleeDebug.GetValueOnGameThread() && Cast<ARPlayer>(GetAvatarActorFromActorInfo()))
	{
		auto* Selected=GetCurrentAttack().Montage.Get();
		auto* Anim=ActorInfo ? ActorInfo->GetAnimInstance() : nullptr;
		UE_LOG(LogTemp,Display,TEXT("MeleeDebug: END %s selected=%s actual=%s position=%.3f duration=%.3f cancelled=%d."),
			*GetName(),*GetNameSafe(Selected),*GetNameSafe(Anim ? Anim->GetCurrentActiveMontage() : nullptr),
			Anim && Selected ? Anim->Montage_GetPosition(Selected) : -1.f,Selected ? Selected->GetPlayLength() : 0.f,bWasCancelled);
	}
	ResetMeleeState();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
