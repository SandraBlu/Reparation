// Fill out your copyright notice in the Description page of Project Settings.


#include "RAssetManager.h"
#include <RGameplayTags.h>

URAssetManager& URAssetManager::Get()
{
	check(GEngine)
	URAssetManager* RAssetManager = Cast<URAssetManager>(GEngine->AssetManager);
	return *RAssetManager;
}

void URAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FRGameplayTags::InitializeNativeGameplayTags();
}
