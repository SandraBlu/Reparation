// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AbilitySystem/Data/RNPCData.h"
#include "RBFL.generated.h"

class URHUDController;
class URAttributeMenuController;
class UAbilitySystemComponent;
class URAbilityMenuController;
struct FWidgetControllerParams;
class URAbilityInfo;
struct FGameplayEffectContextHandle;
struct FGameplayTag;
class ARHUD;

/**
 * 
 */
UCLASS()
class REPARATION_API URBFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	public:

	UFUNCTION(BlueprintPure, Category = "AOBFL|Widget Controller", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParama(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, ARHUD*& OutAOHUD);
	
	UFUNCTION(BlueprintPure, Category = "RBFL|Widget Controller", meta = (DefaultToSelf = "WorldContextObject"))
	static URHUDController* GetOverlayController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "RBFL| Widget Controller", meta = (DefaultToSelf = "WorldContextObject"))
	static URAttributeMenuController* GetAttributeMenuController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "RBFL| Widget Controller", meta = (DefaultToSelf = "WorldContextObject"))
	static URAbilityMenuController* GetAbilityMenuController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "RBFL|NPCClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ENPCClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category="RBFL|NPCClassDefaults")
	static URNPCData* GetCharacterClassInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="RBFL|NPCClassDefaults")
	static URAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="RBFL|NPCClassDefaults")
	static void GiveNPCAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ENPCClass CharacterClass);

	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsDodgedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsDodgedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsDodgedHit);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsSuccessfulDebuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetDebuffDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDamage);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetDebuffDuration(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDuration);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetDebuffFrequency(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InFrequency);

	static void SetDamageType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayMechanics")
	static void GetTargetsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintPure, Category = "AOBFL|GameplayMechanics")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);

	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, ENPCClass CharacterClass, int32 CharacterLevel);

	/*UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static float GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static float GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static float GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static FGameplayTag GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle);*/
	
};
