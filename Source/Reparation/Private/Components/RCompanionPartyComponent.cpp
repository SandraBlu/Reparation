#include "Components/RCompanionPartyComponent.h"
#include "AI/RCompanion.h"
#include "AI/AIC_Companion.h"
#include "AI/REnemy.h"
#include "AI/RAIController.h"
#include "Characters/RCharacterBase.h"
#include "Stealth/RStealthComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GAS/Ability/RDamageAbility.h"
#include "GAS/Ability/RFallDamageAbility.h"
#include "EngineUtils.h"

URCompanionPartyComponent::URCompanionPartyComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.25f;
}

void URCompanionPartyComponent::BindLeaderAbilities()
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner());
	if (BoundASC.Get() == ASC) return;
	if (BoundASC.IsValid()) BoundASC->AbilityActivatedCallbacks.Remove(AbilityActivatedHandle);
	BoundASC = ASC;
	if (ASC) AbilityActivatedHandle = ASC->AbilityActivatedCallbacks.AddUObject(this, &ThisClass::OnLeaderAbilityActivated);
}

void URCompanionPartyComponent::OnLeaderAbilityActivated(UGameplayAbility* Ability)
{
	if (!Ability) return;
	if (Ability->IsA<URFallDamageAbility>()) return;
	if (const URDamageAbility* DamageAbility = Cast<URDamageAbility>(Ability))
	{
		if (!DamageAbility->bSilentAttack) NotifyNormalCombat();
		return;
	}
	const FGameplayTag Melee = FGameplayTag::RequestGameplayTag(TEXT("ability.attack.melee"));
	const FGameplayTag Ranged = FGameplayTag::RequestGameplayTag(TEXT("ability.attack.ranged"));
	if (Ability->GetAssetTags().HasTag(Melee) || Ability->GetAssetTags().HasTag(Ranged)) NotifyNormalCombat();
}

void URCompanionPartyComponent::NotifyNormalCombat(AActor* Threat)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	CombatMemoryRemaining = FMath::Max(0.f, CombatMemoryDuration);
	if (IsValid(Threat) && Threat->Implements<URCombatInterface>() && !IRCombatInterface::Execute_IsDead(Threat) &&
		(Threat->IsA<AREnemy>() || Threat->ActorHasTag(TEXT("enemy"))))
	{
		if (RecentThreat.Get() != Threat) RecentDefender.Reset();
		RecentThreat = Threat;
	}
}

void URCompanionPartyComponent::NotifyReceivedDamage(AActor* Victim, AActor* Attacker)
{
	if (!IsValid(Victim) || !IsValid(Attacker) || !(Attacker->IsA<AREnemy>() || Attacker->ActorHasTag(TEXT("enemy")))) return;
	URCompanionPartyComponent* Party = Victim->FindComponentByClass<URCompanionPartyComponent>();
	if (ARCompanion* Companion = Cast<ARCompanion>(Victim)) Party = Companion->RegisteredParty.Get();
	if (Party)
	{
		Party->NotifyNormalCombat(Attacker);
		Party->RecentDefender = Cast<ARCompanion>(Victim);
	}
}

bool URCompanionPartyComponent::CanAssistNow() const
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return false;
	if (GetOwner()->Implements<URCombatInterface>() && IRCombatInterface::Execute_IsDead(GetOwner())) return false;
	const ACharacter* Leader = Cast<ACharacter>(GetOwner());
	const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(GetOwner());
	const bool bSneaking = (Leader && Leader->bIsCrouched) || (CharacterBase && CharacterBase->Stealth->IsConcealed());
	return !bSneaking || CombatMemoryRemaining > 0.f;
}

bool URCompanionPartyComponent::IsActiveThreat(AActor* Threat) const
{
	const ARCharacterBase* CharacterBase = Cast<ARCharacterBase>(Threat);
	if (!IsValid(Threat) || !Threat->Implements<URCombatInterface>() || IRCombatInterface::Execute_IsDead(Threat) ||
		(CharacterBase && CharacterBase->IsDead_Implementation()) ||
		!(Threat->IsA<AREnemy>() || Threat->ActorHasTag(TEXT("enemy")))) return false;
	if (RecentThreat.Get() == Threat && CombatMemoryRemaining > 0.f) return true;
	AActor* Target = GetThreatTarget(Threat);
	return Target == GetOwner() || (IsValid(Target) && Companions.Contains(Cast<ARCompanion>(Target)));
}

bool URCompanionPartyComponent::IsThreateningCompanion(AActor* Threat, ARCompanion* Companion) const
{
	return IsValid(Companion) && IsActiveThreat(Threat) &&
		(GetThreatTarget(Threat) == Companion || (RecentThreat.Get() == Threat && RecentDefender.Get() == Companion && CombatMemoryRemaining > 0.f));
}

AActor* URCompanionPartyComponent::GetThreatTarget(AActor* Threat) const
{
	if (!IsValid(Threat) || !Threat->Implements<URCombatInterface>()) return nullptr;
	AActor* Target = IRCombatInterface::Execute_GetCombatTarget(Threat);
	if (const AREnemy* Enemy = Cast<AREnemy>(Threat))
	{
		// Perception is authoritative for our enemy controller, including sight loss.
		if (const ARAIController* AI = Cast<ARAIController>(Enemy->GetController()))
			Target = AI->bSearching ? nullptr : AI->GetSightTarget();
		// Native enemy classes can expose an empty reflected interface event.
		else if (!IsValid(Target)) Target = Enemy->GetCombatTarget_Implementation();
	}
	return Target;
}

void URCompanionPartyComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;
	BindLeaderAbilities();
	CombatMemoryRemaining = FMath::Max(0.f, CombatMemoryRemaining - DeltaTime);
	if (Companions.IsEmpty()) return;
	ACharacter* Leader = Cast<ACharacter>(GetOwner());
	if (!Leader) return;
	const ARCharacterBase* LeaderCharacter = Cast<ARCharacterBase>(Leader);
	if ((LeaderCharacter && LeaderCharacter->IsDead_Implementation()) ||
		(Leader->Implements<URCombatInterface>() && IRCombatInterface::Execute_IsDead(Leader))) return;
	TArray<AActor*> Threats;
	if (IsActiveThreat(RecentThreat.Get())) Threats.Add(RecentThreat.Get());
	for (TActorIterator<AREnemy> It(GetWorld()); It; ++It)
		if (IsActiveThreat(*It)) Threats.AddUnique(*It);
	Threats.Sort([Leader](const AActor& A, const AActor& B) { return A.GetDistanceTo(Leader) < B.GetDistanceTo(Leader); });
	for (ARCompanion* Companion : Companions)
	{
		if (!IsValid(Companion) || !Companion->IsCombatReady() || !Companion->bAutomaticAssistance) continue;
		AAIC_Companion* AI = Cast<AAIC_Companion>(Companion->GetController());
		if (!AI || AI->CurrentOrder == ERCompanionOrder::Attack) continue;
		for (AActor* Threat : Threats)
			if (AI->TryAutomaticAssist(Threat, Leader)) break;
	}
}

void URCompanionPartyComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (BoundASC.IsValid()) BoundASC->AbilityActivatedCallbacks.Remove(AbilityActivatedHandle);
	for (ARCompanion* Companion : Companions)
		if (IsValid(Companion)) Companion->RegisteredParty.Reset();
	Super::EndPlay(EndPlayReason);
}

bool URCompanionPartyComponent::AddCompanion(ARCompanion* Companion)
{
	if (!IsValid(Companion) || Companion == GetOwner() || Companions.Contains(Companion) || Companion->RegisteredParty.IsValid()) return false;
	Companions.Add(Companion);
	Companion->RegisteredParty = this;
	BindLeaderAbilities();
	if (!IsValid(SelectedCompanion)) SelectedCompanion = Companion;
	return true;
}

void URCompanionPartyComponent::RemoveCompanion(ARCompanion* Companion)
{
	if (!Companions.Contains(Companion)) return;
	if (IsValid(Companion))
	{
		Companion->RegisteredParty.Reset();
		if (AAIC_Companion* Controller = Cast<AAIC_Companion>(Companion->GetController())) Controller->Stay();
	}
	Companions.Remove(Companion);
	if (SelectedCompanion == Companion) SelectedCompanion = nullptr;
}

bool URCompanionPartyComponent::SelectCompanion(ARCompanion* Companion)
{
	if (!IsValid(Companion) || !Companions.Contains(Companion)) return false;
	SelectedCompanion = Companion;
	return true;
}

bool URCompanionPartyComponent::Follow()
{
	AAIC_Companion* Controller = IsValid(SelectedCompanion) ? Cast<AAIC_Companion>(SelectedCompanion->GetController()) : nullptr;
	return Controller && Controller->FollowLeader(Cast<ACharacter>(GetOwner()));
}

bool URCompanionPartyComponent::Stay()
{
	AAIC_Companion* Controller = IsValid(SelectedCompanion) ? Cast<AAIC_Companion>(SelectedCompanion->GetController()) : nullptr;
	if (!Controller) return false;
	Controller->Stay();
	return true;
}

bool URCompanionPartyComponent::GoToLocation(FVector Location)
{
	AAIC_Companion* Controller = IsValid(SelectedCompanion) ? Cast<AAIC_Companion>(SelectedCompanion->GetController()) : nullptr;
	return Controller && Controller->GoToLocation(Location);
}

bool URCompanionPartyComponent::Attack(AActor* Target)
{
	AAIC_Companion* Controller = IsValid(SelectedCompanion) ? Cast<AAIC_Companion>(SelectedCompanion->GetController()) : nullptr;
	return Controller && Controller->Attack(Target);
}
