#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "REnemyMeleeSweepComponent.generated.h"

/** Mace/head collision sampled throughout an animation-defined strike window. */
UCLASS(ClassGroup=(Combat), meta=(BlueprintSpawnableComponent))
class REPARATION_API UREnemyMeleeSweepComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	UREnemyMeleeSweepComponent();
	void BeginWindow(FGameplayTag EventTag);
	void SampleWindow();
	void EndWindow();
	void ConsumeTargets(TArray<AActor*>& OutTargets);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Sweep")
	bool bUseHeadSweep = false;
	/** Radius of the actual damaging head in cm, not the weapon's reach. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Sweep", meta=(ClampMin="1"))
	float HeadRadius = 15.f;
	/** Move the sphere center from the damage socket to the head's center, in socket space. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Sweep")
	FVector HeadOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Melee Sweep")
	bool bDrawDebug = false;

private:
	bool bWindowOpen = false;
	FVector PreviousHead = FVector::ZeroVector;
	FGameplayTag DamageEventTag;
	TSet<TWeakObjectPtr<AActor>> HitActors;
	TArray<TWeakObjectPtr<AActor>> PendingTargets;
	bool GetHeadLocation(FVector& Location) const;
};
