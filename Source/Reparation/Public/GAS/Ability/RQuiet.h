#pragma once
#include "GAS/Ability/RGameplayAbility.h"
#include "RQuiet.generated.h"

/** Purchased movement-noise technique; always applies at its learned rank. */
UCLASS()
class REPARATION_API URQuiet : public URGameplayAbility
{
    GENERATED_BODY()
public:
    URQuiet();
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Quiet", meta=(ClampMin="0",ClampMax="1"))
    TArray<float> HearingRangeMultipliers = {.8f,.6f,.4f};
    float GetHearingRangeMultiplier(int32 Rank) const;
    virtual FString GetDescription(int32 Level) override;
    virtual FString GetNextLevelDescription(int32 Level) override { return GetDescription(Level); }
};
