// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NavigatorEditorBFL.generated.h"

class UTexture2DFactoryNew;

/**
 * Editor-only counterpart to UNavigatorBFL, for the map tile generation tooling.
 * These live here so the runtime Reparation module never has to link UnrealEd.
 */
UCLASS()
class REPARATIONEDITOR_API UNavigatorEditorBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintPure, Category = "Navigator Function Library")
	static UTexture2DFactoryNew* GetTexture2DFactory();

};
