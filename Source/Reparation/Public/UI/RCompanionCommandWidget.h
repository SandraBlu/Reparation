#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RCompanionCommandWidget.generated.h"

UENUM(BlueprintType)
enum class ERCompanionMenuChoice : uint8 { None, Follow, Stay, GoToLocation, Attack };

/** Native HUD radial. Subclass for presentation; command execution remains in C++. */
UCLASS(Blueprintable)
class REPARATION_API URCompanionCommandWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadOnly, Category="Companion")
	ERCompanionMenuChoice Choice = ERCompanionMenuChoice::None;
	UPROPERTY(BlueprintReadWrite, Category="Companion")
	FText CompanionLabel;
	UPROPERTY(BlueprintReadWrite, Category="Companion")
	FText Feedback;
	UPROPERTY(BlueprintReadWrite, Category="Companion")
	bool bTargeting = false;
	UPROPERTY(BlueprintReadOnly, Category="Companion")
	bool bAttackTargeting = false;
protected:
	virtual void NativeTick(const FGeometry& Geometry, float DeltaSeconds) override;
	virtual int32 NativePaint(const FPaintArgs& Args, const FGeometry& Geometry, const FSlateRect& CullingRect,
		FSlateWindowElementList& DrawElements, int32 LayerId, const FWidgetStyle& Style, bool bParentEnabled) const override;
};
