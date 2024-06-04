// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RWidgetController.generated.h"

class UAttributeSet;
class UAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API URWidgetController : public UObject
{
	GENERATED_BODY()

protected:

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	APlayerController* PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	APlayerState* PlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	UAttributeSet* AttributeSet;
	
};
