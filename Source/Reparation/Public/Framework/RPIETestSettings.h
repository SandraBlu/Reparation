#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "RPIETestSettings.generated.h"

/** Temporary progression for fresh PIE sessions. Never applied to packaged games. */
UCLASS(Config=Editor, DefaultConfig, meta=(DisplayName="PIE Testing"))
class REPARATION_API URPIETestSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override { return TEXT("Game"); }

	UPROPERTY(Config, EditAnywhere, Category="Player", meta=(DisplayName="Enable Test Overrides", ToolTip="Applies on the next PIE session. Disable and restart PIE for normal progression."))
	bool bEnableTestOverrides = false;

	UPROPERTY(Config, EditAnywhere, Category="Player", meta=(EditCondition="bEnableTestOverrides", ClampMin="1", ClampMax="40"))
	int32 StartingLevel = 25;

	UPROPERTY(Config, EditAnywhere, Category="Player", meta=(EditCondition="bEnableTestOverrides", ClampMin="0"))
	int32 StartingSkillPoints = 30;

	UPROPERTY(Config, EditAnywhere, Category="Player", meta=(EditCondition="bEnableTestOverrides", ClampMin="0"))
	int32 StartingAttributePoints = 10;

	UPROPERTY(Config, EditAnywhere, Category="Player", meta=(EditCondition="bEnableTestOverrides", ClampMin="0", ToolTip="Temporary inventory currency for testing branch resets; applied once per fresh PIE player."))
	int32 StartingGold = 500;
};
