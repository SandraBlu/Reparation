// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/UserWidget.h"
#include "RUserWidget.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URUserWidget : public UCommonUserWidget
{
	GENERATED_BODY()

public:

	virtual void NativeConstruct() override;
	
	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);
	
	UPROPERTY(BlueprintReadOnly)
	UObject* WidgetController;
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Notifications")
	void ShowNotification(const FText& NotificationText, const float Duration = 5.f);
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerSet();
};
