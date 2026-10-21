#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RCompanionPartyComponent.generated.h"

class ARCompanion;
class UAbilitySystemComponent;
class UGameplayAbility;

/** Player-owned roster and selection; does not create companion inventories. */
UCLASS(ClassGroup=(Reparation), meta=(BlueprintSpawnableComponent))
class REPARATION_API URCompanionPartyComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	URCompanionPartyComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** Normal combat releases sneaking suppression briefly; silent attacks must not call this. */
	UFUNCTION(BlueprintCallable, Category="Companion|Assistance")
	void NotifyNormalCombat(AActor* Threat = nullptr);
	static void NotifyReceivedDamage(AActor* Victim, AActor* Attacker);
	bool IsActiveThreat(AActor* Threat) const;
	bool IsThreateningCompanion(AActor* Threat, ARCompanion* Companion) const;
	bool CanAssistNow() const;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Assistance", meta=(ClampMin="0"))
	float CombatMemoryDuration = 6.f;
	UFUNCTION(BlueprintCallable, Category="Companion|Party")
	bool AddCompanion(ARCompanion* Companion);
	UFUNCTION(BlueprintCallable, Category="Companion|Party")
	void RemoveCompanion(ARCompanion* Companion);
	UFUNCTION(BlueprintCallable, Category="Companion|Party")
	bool SelectCompanion(ARCompanion* Companion);
	UFUNCTION(BlueprintCallable, Category="Companion|Commands")
	bool Follow();
	UFUNCTION(BlueprintCallable, Category="Companion|Commands")
	bool Stay();
	UFUNCTION(BlueprintCallable, Category="Companion|Commands")
	bool GoToLocation(FVector Location);
	UFUNCTION(BlueprintCallable, Category="Companion|Commands")
	bool Attack(AActor* Target);
	UPROPERTY(BlueprintReadOnly, Category="Companion|Party")
	TArray<TObjectPtr<ARCompanion>> Companions;
	UPROPERTY(BlueprintReadOnly, Category="Companion|Party")
	TObjectPtr<ARCompanion> SelectedCompanion;
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	AActor* GetThreatTarget(AActor* Threat) const;
	TWeakObjectPtr<ARCompanion> RecentDefender;
	void OnLeaderAbilityActivated(UGameplayAbility* Ability);
	void BindLeaderAbilities();
	TWeakObjectPtr<UAbilitySystemComponent> BoundASC;
	FDelegateHandle AbilityActivatedHandle;
	TWeakObjectPtr<AActor> RecentThreat;
	float CombatMemoryRemaining = 0.f;
};
