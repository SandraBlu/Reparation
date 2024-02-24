// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AOInteractWidget.generated.h"

UCLASS()
class REPARATION_API UAOInteractWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Interact")
	void UpdateInteractWidget(class UAOInteractComponent* InteractComp);

	UFUNCTION(BlueprintImplementableEvent)
	void OnUpdateInteractWidget();

	UPROPERTY(BlueprintReadOnly, Category = "Interact", meta = (ExposeOnSpawn))
	class UAOInteractComponent* OwningInteractComp;
	
};
