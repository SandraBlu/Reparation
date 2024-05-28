// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RHUD.generated.h"

struct FWidgetControllerParams;
class UAbilitySystemComponent;
class UAttributeSet;
class URUserWidget;
class UROverlayController;

/**
 * 
 */
UCLASS()
class REPARATION_API ARHUD : public AHUD
{
	GENERATED_BODY()

public:

	UPROPERTY()
	URUserWidget* OverlayWidget;
	
	UROverlayController* GetOverlayController(const FWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<URUserWidget> OverlayWidgetClass;

	UPROPERTY()
	UROverlayController* OverlayController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UROverlayController> OverlayControllerClass;
};
