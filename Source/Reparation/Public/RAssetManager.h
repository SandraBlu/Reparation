// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "RAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:

	static URAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;
	
};
