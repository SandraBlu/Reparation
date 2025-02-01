// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "RAbilityTypes.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/Data/RCharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "RTypes/REnumTypes.h"
#include "UI/GAS/RHUD.h"
#include "RAbilitySystemLibrary.generated.h"

class UAbilityInfo;
class URAbilityMenuController;
class UAbilitySystemComponent;
class URAttributeMenuController;
class UROverlayWidgetController;
struct FWidgetControllerParams;
class UPawnCombatComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API URAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	//Widget Controllers
	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static bool MakeWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, ARHUD*& OutRHUD);

	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static UROverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static URAttributeMenuController* GetAttributeMenuController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "RBFL|WidgetController", meta = (DefaultToSelf = "WorldContextObject"))
	static URAbilityMenuController* GetAbilityMenuController(const UObject* WorldContextObject);
	
	//Initializers
	UFUNCTION(BlueprintCallable, Category = "RBFL|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC);

	UFUNCTION(BlueprintCallable, Category = "RBFL|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass);
	//Class/AbilityInfo
	UFUNCTION(BlueprintCallable, Category="RBFL|CharacterClassDefaults")
	static URCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="RBFL|CharacterClassDefaults")
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	//Getters Damage Parameters
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
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static bool IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static float GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static float GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle);
	UFUNCTION(BlueprintPure, Category = "RBFL|GameplayEffects")
	static FVector GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle);

	//Setters Damage Parameters
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
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetIsRadialDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetRadialDamageInnerRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InInnerRadius);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetRadialDamageOuterRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InOuterRadius);
	UFUNCTION(BlueprintCallable, Category = "RBFL|GameplayEffects")
	static void SetRadialDamageOrigin(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle,const FVector& InOrigin);
	

	//Gameplay Ability Functions
	UFUNCTION(BlueprintCallable, Category = "AOBFL|GAMelee")
	static void GetTargetsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintPure, Category = "AOBFL|GAMelee")
	static bool IsNotFriend(AActor* FirstActor, AActor* SecondActor);

	static int32 GetXPRewardForEnemySlay(const UObject* WorldContextObject, ECharacterClass CharacterClass, int32 CharacterLevel);

	UFUNCTION(BlueprintCallable, Category = "RBFL|DamageEffect")
	static FGameplayEffectContextHandle ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams);

	//Target Ability
	UFUNCTION(BlueprintPure, Category = "RBFL|GAS")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);
	UFUNCTION(BlueprintPure, Category = "RBFL|GAS")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);

	//Damage Effect Params
	UFUNCTION(BlueprintCallable, Category = "RBFL|DamageEffect")
	static void SetIsRadialDamageEffectParam(UPARAM(ref)FDamageEffectParams& DamageEffectParams, bool bIsRadial, float InnerRadius, float OuterRadius, FVector Origin);
	
	UFUNCTION(BlueprintCallable, Category = "RBFL|DamageEffect")
	static void SetKnockbackDirection(UPARAM(ref)FDamageEffectParams& DamageEffectParams, FVector KnockbackDirection, float Magnitude = 0.f);

	UFUNCTION(BlueprintCallable, Category = "RBFL|DamageEffect")
	static void SetDeathImpulseDirection(UPARAM(ref)FDamageEffectParams& DamageEffectParams, FVector ImpulseDirection, float Magnitude = 0.f);

	UFUNCTION(BlueprintCallable, Category = "RBFL|DamageEffect")
	static void SetTargetEffectParamsASC(UPARAM(ref)FDamageEffectParams& DamageEffectParams, UAbilitySystemComponent* InASC);

	//--- added Melee CombatFunctions

	static URAbilitySystemComponent* NativeGetRASCFromActor(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "RBFL")
	static void AddGameplayTagToActorIfNone(AActor* InActor,FGameplayTag TagToAdd);

	UFUNCTION(BlueprintCallable, Category = "RBFL")
	static void RemoveGameplayTagFromActorIfFound(AActor* InActor,FGameplayTag TagToRemove);
	
	static bool NativeDoesActorHaveTag(AActor* InActor,FGameplayTag TagToCheck);

	UFUNCTION(BlueprintCallable, Category = "RBFL", meta = (DisplayName = "Does Actor Have Tag", ExpandEnumAsExecs = "OutConfirmType"))
	static void BP_DoesActorHaveTag(AActor* InActor,FGameplayTag TagToCheck,ERConfirmType& OutConfirmType);
	
	static UPawnCombatComponent* NativeGetPawnCombatComponentFromActor(AActor* InActor);

	UFUNCTION(BlueprintCallable, Category = "RBFL", meta = (DisplayName = "Get Pawn Combat Component From Actor", ExpandEnumAsExecs = "OutValidType"))
	static UPawnCombatComponent* BP_GetPawnCombatComponentFromActor(AActor* InActor,ERValidType& OutValidType);

	UFUNCTION(BlueprintPure, Category = "RBFL", meta = (CompactNodeTitle = "Get Value At Level"))
	static float GetScalableFloatValueAtLevel(const FScalableFloat& ScalableFloat, float InLevel = 1.f);

	UFUNCTION(BlueprintPure, Category = "RBFL", meta = (CompactNodeTitle = "Get Value At Level"))
	static FGameplayTag ComputeHitReactDirection(AActor* InAttacker, AActor* InTarget, float& HitDirection);

	UFUNCTION(BlueprintPure, Category = "RBFL")
	static bool IsValidBlock(AActor* InAttacker, AActor* InDefender);
};
