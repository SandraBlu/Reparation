// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RHUD.generated.h"

class UROverlayWidgetController;
class UAttributeSet;
class UAbilitySystemComponent;
struct FWidgetControllerParams;
class URUserWidget;
/**
 * 
 */
UCLASS()
class REPARATION_API ARHUD : public AHUD
{
	GENERATED_BODY()

public:

	UROverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);
	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:
	
	UPROPERTY()
	URUserWidget* OverlayWidget;
	UPROPERTY(EditAnywhere)
	TSubclassOf<URUserWidget> OverlayWidgetClass;
	
	UPROPERTY()
	UROverlayWidgetController* OverlayWidgetController;
	UPROPERTY(EditAnywhere)
	TSubclassOf<UROverlayWidgetController> OverlayWidgetControllerClass;
	
};
