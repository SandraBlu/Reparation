#pragma once

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
















































































