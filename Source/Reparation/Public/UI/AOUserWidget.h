// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOUserWidget.generated.h"


UCLASS()
class REPARATION_API UAOUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable)
	void SetWidgetController(UObject* InWidgetController);

	UPROPERTY(BlueprintReadOnly)
	UObject* WidgetController;

protected:

	UFUNCTION(BlueprintImplementableEvent)
	void WidgetControllerInit();

	
};
