// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "AOHUD.generated.h"

class UAOUserWidget;
class UOverlayWidgetController;
struct FWidgetControllerParams;
class APlayerController;
class APlayerState;
class UAbilitySystemComponent;
class UAttributeSet;

UCLASS()
class REPARATION_API AAOHUD : public AHUD
{
	GENERATED_BODY()

public:

	UPROPERTY()
	UAOUserWidget* OverlayWidget;

	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WCParams);

	void InitOverlay(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS);

private:
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<UAOUserWidget> OverlayWidgetClass;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY()
	UOverlayWidgetController* OverlayWidgetController;
};
