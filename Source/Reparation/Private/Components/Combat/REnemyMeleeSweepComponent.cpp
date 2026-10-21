#include "Components/Combat/REnemyMeleeSweepComponent.h"
#include "AI/REnemy.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "Framework/RAbilitySystemLibrary.h"

UREnemyMeleeSweepComponent::UREnemyMeleeSweepComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UREnemyMeleeSweepComponent::GetHeadLocation(FVector& Location) const
{
	const AREnemy* Enemy = Cast<AREnemy>(GetOwner());
	USkeletalMeshComponent* Mesh = Enemy ? Enemy->GetMeleeDamageMesh() : nullptr;
	if (!Mesh || !Mesh->DoesSocketExist(Enemy->GetMeleeDamageSocket())) return false;
	Location = Mesh->GetSocketTransform(Enemy->GetMeleeDamageSocket()).TransformPosition(HeadOffset);
	return !Location.ContainsNaN();
}

void UREnemyMeleeSweepComponent::BeginWindow(FGameplayTag EventTag)
{
	EndWindow();
	if (!bUseHeadSweep || !GetOwner()->HasAuthority()) return;
	if (!EventTag.IsValid() || !GetHeadLocation(PreviousHead))
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy melee window unavailable for %s: verify its weapon damage socket and damage-event tag."), *GetNameSafe(GetOwner()));
		return;
	}
	DamageEventTag = EventTag;
	bWindowOpen = true;
	SampleWindow();
}

void UREnemyMeleeSweepComponent::SampleWindow()
{
	if (!bWindowOpen || !GetWorld() || !GetOwner()->HasAuthority()) return;
	const AREnemy* Enemy = Cast<AREnemy>(GetOwner());
	FVector Head;
	if (!Enemy || Enemy->IsDead_Implementation() || !GetHeadLocation(Head)) { EndWindow(); return; }
	FCollisionQueryParams Params(SCENE_QUERY_STAT(EnemyMeleeHead), false, GetOwner());
	FCollisionObjectQueryParams Objects;
	Objects.AddObjectTypesToQuery(ECC_Pawn);
	TArray<FHitResult> Hits;
	const float Radius = FMath::IsFinite(HeadRadius) ? FMath::Max(1.f, HeadRadius) : 15.f;
	GetWorld()->SweepMultiByObjectType(Hits, PreviousHead, Head, FQuat::Identity, Objects, FCollisionShape::MakeSphere(Radius), Params);
	if (bDrawDebug)
	{
		DrawDebugLine(GetWorld(), PreviousHead, Head, FColor::Yellow, false, 0.15f);
		DrawDebugSphere(GetWorld(), Head, Radius, 12, FColor::Red, false, 0.15f);
	}
	PreviousHead = Head;
	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!IsValid(Target) || HitActors.Contains(Target) || !Target->Implements<URCombatInterface>() ||
			IRCombatInterface::Execute_IsDead(Target) || !URAbilitySystemLibrary::IsNotFriend(GetOwner(), Target) ||
			!UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target)) continue;
		HitActors.Add(Target);
		PendingTargets.Add(Target);
	}
	if (!PendingTargets.IsEmpty())
	{
		FGameplayEventData Event;
		Event.Instigator = GetOwner();
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetOwner(), DamageEventTag, Event);
	}
}

void UREnemyMeleeSweepComponent::ConsumeTargets(TArray<AActor*>& OutTargets)
{
	OutTargets.Reset();
	for (const TWeakObjectPtr<AActor>& Target : PendingTargets)
		if (Target.IsValid()) OutTargets.AddUnique(Target.Get());
	PendingTargets.Reset();
}

void UREnemyMeleeSweepComponent::EndWindow()
{
	bWindowOpen = false;
	HitActors.Reset();
	PendingTargets.Reset();
	DamageEventTag = FGameplayTag();
}
