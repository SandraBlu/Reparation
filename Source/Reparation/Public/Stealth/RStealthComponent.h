#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RStealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRConcealmentChanged, bool, bConcealed);

/** Shared player/companion vegetation concealment; evaluated from current position and stance. */
UCLASS(ClassGroup=(Stealth), meta=(BlueprintSpawnableComponent))
class REPARATION_API URStealthComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	URStealthComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintPure, Category="Stealth")
	bool IsInSneakArea() const;

	UFUNCTION(BlueprintPure, Category="Stealth") 
	bool IsConcealed() const;

	/** Cover blocks sight at any distance, including for previously spotted targets. */
	bool IsConcealedFrom(const FVector& ObserverLocation, bool bPreviouslyVisible) const;

	UPROPERTY(BlueprintAssignable, Category="Stealth")
	FRConcealmentChanged OnConcealmentChanged;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Stealth")
	bool bConcealmentEnabled = true;

private:
	bool bLastConcealed = false;
	bool FindConcealment() const;
};
