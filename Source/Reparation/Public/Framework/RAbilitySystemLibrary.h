// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectTypes.h"
#include "RAbilityTypes.h"
#include "GAS/Data/RCharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/GAS/RHUD.h"
#include "RAbilitySystemLibrary.generated.h"

class UAbilityInfo;
class URAbilityMenuController;
class UAbilitySystemComponent;
class URAttributeMenuController;
class UROverlayWidgetController;
struct FWidgetControllerParams;
/**
 * 
 */
UCLASS()
class REPARATION_API URAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, ARHUD*& OutRHUD);

	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UROverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static URAttributeMenuController* GetAttributeMenuController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static URAbilityMenuController* GetAbilityMenuController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "RBFL|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "RBFL|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);
	
	UFUNCTION(BlueprintCallable, Category="RBFL|CharacterClassDefaults")
	static URCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="RBFL|CharacterClassDefaults")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);
	
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsDodgedHit(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static float GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static float GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static float GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static FGameplayTag GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static FVector GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static FVector GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsDodgedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsDodgedHit);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsSucessfulDebuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetDebuffDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InDebuffDamage);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetDebuffDuration(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float bInDebuffDuration);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetDebuffFrequency(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float bInDebuffFrequency);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetDamageType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle,const FGameplayTag& InDamageType);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetDeathImpulse(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle,const FVector& InImpulse);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetKnockbackForce(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle,const FVector& InForce);

	UFUNCTION(BlueprintCallable, Category = "AOBFL|GAMelee")
	static void GetTargetsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintPure, Category = "AOBFL|GAMelee")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);

	static int32 GetXPRewardForEnemySlay(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);

	UFUNCTION(BlueprintCallable, Category = "RBFL|DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	UFUNCTION(BlueprintPure, Category = "RBFL|GAS")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);
	UFUNCTION(BlueprintPure, Category = "RBFL|GAS")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);
};
