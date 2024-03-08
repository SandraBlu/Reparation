// Fill out your copyright notice in the Description page of Project Settings.


#include "AOAssetManager.h"
#include "AOGameplayTags.h"


UAOAssetManager& UAOAssetManager::Get()
{
	check(GEngine);
	UAOAssetManager* AOAssetManager = Cast<UAOAssetManager>(GEngine->AssetManager);
	return *AOAssetManager;
}

void UAOAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FAOGameplayTags::InitializeNativeGameplayTags();

}
