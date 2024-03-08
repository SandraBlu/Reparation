// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "AOAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UAOAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	static UAOAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;


};
