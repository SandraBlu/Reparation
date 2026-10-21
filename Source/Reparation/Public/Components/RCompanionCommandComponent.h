#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputCoreTypes.h"
#include "RCompanionCommandComponent.generated.h"

class UEnhancedInputComponent;
class UEnhancedInputLocalPlayerSubsystem;
class UInputAction;
class UInputMappingContext;
class URCompanionCommandWidget;
class ARCompanion;

/** Local input and HUD lifecycle for companion orders. */
UCLASS(ClassGroup=(Reparation), meta=(BlueprintSpawnableComponent))
class REPARATION_API URCompanionCommandComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	URCompanionCommandComponent();
	void SetupInput(UEnhancedInputComponent* Input);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	bool IsMenuOpen() const { return bMenuOpen; }
	bool IsCommandInputActive() const { return bMenuOpen || bTargeting; }
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Input")
	FKey MenuKey = EKeys::V;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Companion|UI")
	TSubclassOf<URCompanionCommandWidget> WidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Targeting", meta=(ClampMin="100"))
	float TargetRange = 10000.f;
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY(Transient) TObjectPtr<UInputMappingContext> OpenContext;
	UPROPERTY(Transient) TObjectPtr<UInputMappingContext> ActiveContext;
	UPROPERTY(Transient) TObjectPtr<UInputAction> OpenAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> CancelAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> ConfirmAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> NextAction;
	UPROPERTY(Transient) TObjectPtr<UInputAction> PreviousAction;
	UPROPERTY(Transient) TObjectPtr<URCompanionCommandWidget> Widget;
	UPROPERTY(Transient) TObjectPtr<ARCompanion> TargetRecipient;
	UPROPERTY(Transient) TObjectPtr<ARCompanion> MarkerRecipient;
	TWeakObjectPtr<UEnhancedInputComponent> BoundInput;
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> InputSubsystem;
	TWeakObjectPtr<APlayerController> MenuController;
	TArray<uint32> BindingHandles;
	bool bMenuOpen = false;
	bool bTargeting = false;
	bool bAttackTargeting = false;
	bool bSavedCursor = false;
	bool bMovementBlocked = false;
	bool bLookBlocked = false;
	void OpenMenu();
	void ReleaseMenu();
	void Cancel();
	void ConfirmTarget();
	void NextCompanion();
	void PreviousCompanion();
	void CycleCompanion(int32 Direction);
	void UpdateLabel();
};
