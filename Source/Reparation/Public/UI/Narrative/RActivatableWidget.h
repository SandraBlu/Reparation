// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "RActivatableWidget.generated.h"


DECLARE_DYNAMIC_DELEGATE_OneParam(FInputActionExecutedDelegate, FName, ActionName);

USTRUCT(BlueprintType)
struct FInputActionBindingHandle
{
	GENERATED_BODY()
	
public:
	struct FUIActionBindingHandle Handle;
};
/**
 * 
 */
UCLASS(meta = (DisableNativeTick))
class REPARATION_API URActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Narrative Activatable Widget")
	void RegisterBinding(FDataTableRowHandle InputAction, const FInputActionExecutedDelegate& Callback, FInputActionBindingHandle& BindingHandle, FText OverrideDisplayName, const bool bShouldDisplayInActionBar=true);

	UFUNCTION(BlueprintCallable, Category = "Narrative Activatable Widget")
	void UnregisterBinding(FInputActionBindingHandle BindingHandle);

	UFUNCTION(BlueprintCallable, Category = "Narrative Activatable Widget")
	void UnregisterAllBindings();

	UFUNCTION(BlueprintCallable, Category = "Narrative Activatable Widget")
	void SetBindingDisplayName(FInputActionBindingHandle BindingHandle, FText NewDisplayName);

protected:


private:
	TArray<struct FUIActionBindingHandle> BindingHandles;
	
};
