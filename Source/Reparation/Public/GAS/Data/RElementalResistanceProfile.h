#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RElementalResistanceProfile.generated.h"

USTRUCT(BlueprintType)
struct FRElementalResistanceBand
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) int32 MinimumEnemyLevel = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TMap<FGameplayTag, float> DamageMultipliers;
};
/** Assigned per enemy type; levels only modify types explicitly authored in a band. */
UCLASS(BlueprintType)
class REPARATION_API URElementalResistanceProfile : public UDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FText EnemyType;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TMap<FGameplayTag, float> DamageMultipliers;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FRElementalResistanceBand> LevelBands;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) bool bImmuneToControl = false;
    UFUNCTION(BlueprintPure) float GetDamageMultiplier(FGameplayTag DamageType, int32 EnemyLevel) const
    {
        const float* Value = DamageMultipliers.Find(DamageType);
        int32 BestLevel = 0;
        for (const FRElementalResistanceBand& Band : LevelBands)
            if (EnemyLevel >= Band.MinimumEnemyLevel && Band.MinimumEnemyLevel >= BestLevel)
                if (const float* Override = Band.DamageMultipliers.Find(DamageType)) { Value = Override; BestLevel = Band.MinimumEnemyLevel; }
        return Value && FMath::IsFinite(*Value) ? FMath::Clamp(*Value, 0.f, 3.f) : 1.f;
    }
};
