// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "RCommonUISubsystem.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URCommonUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public: 	

	//Cached common hud 
	UPROPERTY(BlueprintReadOnly, Category = "Narrative Common UI")
	class URUserWidget* CommonHUD;

	//Send a notification to the narrative HUD
	UFUNCTION(BlueprintCallable, Category = "Narrative Common UI")
	void ShowNotification(const FText& NotificationText, const float Duration = 5.f);
	
};
