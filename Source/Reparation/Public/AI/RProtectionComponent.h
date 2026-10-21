#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RProtectionComponent.generated.h"
UENUM(BlueprintType)
enum class ERWorldRole : uint8 { Combatant, Guard, Patrol, Sentry, Civilian, Companion };
UENUM(BlueprintType)
enum class ERCombatRole : uint8 { Melee, Heavy, Skirmisher, Ranged, Support };
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FProtectionWarning, AActor*, Intruder, FText, Warning);
UCLASS(ClassGroup=(AI), meta=(BlueprintSpawnableComponent))
class REPARATION_API URProtectionComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    URProtectionComponent();
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Roles") ERWorldRole WorldRole = ERWorldRole::Combatant;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Roles") ERCombatRole CombatRole = ERCombatRole::Melee;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Protection", meta=(ClampMin="1")) float ProtectionRadius = 1200.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Protection", meta=(ClampMin="0")) float WarningGraceSeconds = 4.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Protection") bool bWarnBeforeAttack = true;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Protection") FText WarningText = FText::FromString(TEXT("Leave this area."));
    UPROPERTY(BlueprintAssignable, Category="Protection") FProtectionWarning OnProtectionWarning;
    UPROPERTY(BlueprintReadOnly, Category="Protection") FVector ProtectionCenter = FVector::ZeroVector;
    UFUNCTION(BlueprintCallable, Category="Protection") void SetProtectionCenter(FVector Center);
    UFUNCTION(BlueprintPure, Category="Protection") bool CanEngage(AActor* Actor) const;
    void NotifyProvoked(AActor* Actor);
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
    TMap<TWeakObjectPtr<AActor>, double> WarningStarts;
    TSet<TWeakObjectPtr<AActor>> Provoked;
    bool IsVisibleIntruder(AActor* Actor) const;
    UFUNCTION(NetMulticast, Reliable) void MulticastWarning(AActor* Intruder, const FText& Text);
};
