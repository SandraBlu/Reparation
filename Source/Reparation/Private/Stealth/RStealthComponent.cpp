#include "Stealth/RStealthComponent.h"
#include "Stealth/RSneakArea.h"
#include "EngineUtils.h"
#include "GameFramework/Character.h"

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
