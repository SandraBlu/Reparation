// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Controller/RWidgetController.h"
#include "GameFramework/HUD.h"
#include "RHUD.generated.h"

class URUserWidget;
class UROverlayController;
struct FWidgetControllerParams;
class UAttributeSet;
class UAbilitySystemComponent;

/**
 * 
 */
UCLASS()
class REPARATION_API ARHUD : public AHUD
{
	GENERATED_BODY()
	
public:
	
	UROverlayController* GetOverlayController(const FWidgetControllerParams& WCParams);
	
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:
	
	UPROPERTY()
	URUserWidget*  OverlayWidget;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<URUserWidget> OverlayWidgetClass;

	UPROPERTY()
	UROverlayController* OverlayController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UROverlayController> OverlayControllerClass;

	
};
