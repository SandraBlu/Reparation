#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RTypes/REnumTypes.h"
#include "RCombatBalanceSettings.generated.h"

USTRUCT(BlueprintType)
struct FRCombatLevelBand
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) int32 MinimumPlayerLevel = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1", ClampMax="6")) int32 PlayerAttackers = 2;
};

USTRUCT(BlueprintType)
struct REPARATION_API FRCombatDifficultyProfile
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1", ClampMax="6")) int32 PlayerAttackers = 2;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1", ClampMax="6")) int32 CompanionAttackers = 1;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.1", ClampMax="5")) float EnemyDamageMultiplier = 1.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0")) float TargetCommitmentSeconds = 3.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="1")) float TargetSwitchRatio = 1.25f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0")) float ThreatDecayPerSecond = 5.f;
    /** Optional level gates; an empty array keeps a fixed difficulty-based cap. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly) TArray<FRCombatLevelBand> PlayerLevelBands;
    int32 GetPlayerAttackerLimit(int32 Level) const;
};

UCLASS(BlueprintType)
class REPARATION_API URCombatBalanceSettings : public UDataAsset
{
    GENERATED_BODY()
public:
    URCombatBalanceSettings();
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FRCombatDifficultyProfile Easy;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FRCombatDifficultyProfile Normal;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FRCombatDifficultyProfile Hard;
    UPROPERTY(EditAnywhere, BlueprintReadOnly) FRCombatDifficultyProfile VeryHard;
    FRCombatDifficultyProfile GetProfile(ERGameDifficulty Difficulty) const;
};
