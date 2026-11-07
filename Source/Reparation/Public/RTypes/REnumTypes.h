#pragma once

#include "CoreMinimal.h"
#include "REnumTypes.generated.h"   // must be the last include

//Enums and Structs
class UInputMappingContext;
class URGameplayAbility;

UENUM()
enum class ERConfirmType : uint8
{
	Yes,
	No
};

UENUM()
enum class ERValidType : uint8
{
	Valid,
	Invalid
};

UENUM(BlueprintType)
enum class EFoot : uint8
{
	Left  UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UENUM()
enum class ERCountDownActionInput : uint8
{
	Start,
	Cancel
};
UENUM()
enum class ERCountDownActionOutput : uint8
{
	Updated,
	Completed,
	Cancelled
};

UENUM(BlueprintType)
enum class ERGameDifficulty : uint8
{
	Easy,
	Normal,
	Hard,
	VeryHard
};

UENUM(BlueprintType)
enum class ERXPSource : uint8
{
	Main, Dungeon, MajorSide, MinorSide, Teleport,
	Collectible, RareCollectible, Combat, Companion,
	Secret, LoreSet, Discovery
};

USTRUCT(BlueprintType)
struct FRSourceXP
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) ERXPSource Source = ERXPSource::Main;

	// 10 entries, index 0 = chapter 1
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) TArray<int32> XPByChapter;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly) bool bCatchUpEligible = false;
};








































































