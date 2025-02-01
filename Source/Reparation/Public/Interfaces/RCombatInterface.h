
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GAS/Data/RCharacterClassInfo.h"
#include "UObject/Interface.h"
#include "RCombatInterface.generated.h"

class UPawnCombatComponent;
class ARWeapon;
class UNiagaraSystem;
class UAbilitySystemComponent;
class RCharacterBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnASCRegistered, UAbilitySystemComponent*)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDeath, AActor*, DeadActor);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnDamageSignature, float);

USTRUCT(BlueprintType)
struct FTaggedMontage
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* Montage = nullptr;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag MontageTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FGameplayTag SocketTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* ImpactSound = nullptr;
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class URCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class REPARATION_API IRCombatInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintNativeEvent)
	void GetHit(const FVector& ImpactPoint);
	
	UFUNCTION(BlueprintNativeEvent)
	int32 GetPlayerLevel();

	virtual UPawnCombatComponent* GetPawnCombatComponent() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	FVector GetCombatSocketLocation(const FGameplayTag& CombatSocketTag);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetHitReactMontage();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable)
	UAnimMontage* GetStunnedMontage();
	
	virtual void Die(const FVector& DeathImpulse) = 0;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	bool IsDead() const;

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void FaceTarget(const FVector& Target);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetAvatar();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	void SetCombatTarget(AActor* InCombatTarget);
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetCombatTarget() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	TArray<FTaggedMontage> GetAttackMontages();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	UNiagaraSystem* GetBloodEffect();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	ECharacterClass GetCharacterClass();
	
	virtual FOnASCRegistered GetOnASCRegisteredDelegate() = 0;
	virtual FOnDeath GetOnDeathDelegate() = 0;
	virtual FOnDamageSignature& GetOnDamageSignature() = 0;
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent)
	AActor* GetCurrentEquippedWeapon(ARWeapon* InWeapon);
	
	};
