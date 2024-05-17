// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "RHUD.generated.h"

class URUserWidget;
struct FWidgetControllerParams;
class UAbilitySystemComponent;
class UAttributeSet;
class URHUDController;

/**
 * 
 */
UCLASS()
class REPARATION_API ARHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	

	URHUDController* GetOverlayController(const FWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:

	UPROPERTY()
	URUserWidget*  OverlayWidget;

	UPROPERTY(EditAnywhere)
	TSubclassOf<URUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<URHUDController> HUDController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<URHUDController> HUDControllerClass;
};
