#include "Stealth/RStealthComponent.h"
#include "Stealth/RSneakArea.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Ability/RQuiet.h"

float URStealthComponent::GetMovementNoiseRangeMultiplier() const
{
	auto* ASC=Cast<URAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()));
	if (!ASC) return 1.f;
	const auto Tag=FGameplayTag::RequestGameplayTag(TEXT("ability.stealth.quiet"));
	const int32 Rank=ASC->GetLearnedSkillRank(Tag);
	const auto* Spec=ASC->GetSpecFromAbilityTag(Tag);
	const auto* Quiet=Spec ? Cast<URQuiet>(Spec->Ability) : nullptr;
	return Quiet ? Quiet->GetHearingRangeMultiplier(Rank) : 1.f;
}

void URStealthComponent::RecordNoise(float Loudness)
{
	if (GetWorld() && Loudness > 0.f) LastNoiseTime=GetWorld()->GetTimeSeconds();
}

bool URStealthComponent::IsQuiet(float MaximumSpeed,float QuietSeconds) const
{
	return GetOwner() && GetWorld() && GetOwner()->GetVelocity().Size2D() <= FMath::Max(0.f,MaximumSpeed)
		&& GetWorld()->GetTimeSeconds()-LastNoiseTime >= FMath::Max(0.f,QuietSeconds);
}

URStealthComponent::URStealthComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 0.2f;
}

bool URStealthComponent::IsInSneakArea() const
{
	if (!GetOwner() || !GetWorld()) return false;
	for (TActorIterator<ARSneakArea> It(GetWorld()); It; ++It)
		if (It->ContainsLocation(GetOwner()->GetActorLocation())) return true;
	return false;
}

bool URStealthComponent::FindConcealment() const
{
	const ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (!bConcealmentEnabled || !Character || !GetWorld()) return false;
	for (TActorIterator<ARSneakArea> It(GetWorld()); It; ++It)
	{
		if (It->ContainsLocation(Character->GetActorLocation()) && (!It->bRequiresCrouch || Character->bIsCrouched))
		{
			return true;
		}
	}
	return false;
}

bool URStealthComponent::IsConcealed() const
{
	return FindConcealment();
}

bool URStealthComponent::IsConcealedFrom(const FVector& ObserverLocation, bool bPreviouslyVisible) const
{
	return IsConcealed();
}

void URStealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	const bool bConcealed = IsConcealed();
	if (bConcealed != bLastConcealed)
	{
		bLastConcealed = bConcealed;
		OnConcealmentChanged.Broadcast(bConcealed);
	}
}
