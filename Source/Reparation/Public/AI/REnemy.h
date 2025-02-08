// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/RCharacterBase.h"
#include "UI/GAS/Controllers/ROverlayWidgetController.h"
#include "REnemy.generated.h"

class UTextBlock;
class UREnemyCombatComponent;
class UPawnSensingComponent;
class ARAIController;
class UBehaviorTree;
class UWidgetComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API AREnemy : public ARCharacterBase
{
	GENERATED_BODY()

public:

	AREnemy();

	virtual void PossessedBy(AController* NewController) override;

	//Combat Interface
	virtual UPawnCombatComponent* GetPawnCombatComponent() const override;
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag) override;
	virtual void Die(const FVector& DeathImpulse) override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;


	FORCEINLINE UREnemyCombatComponent* GetEnemyCombatComponent() const { return EnemyCombatComp;}
	
	UFUNCTION(NetMulticast, Reliable)
	virtual void MulticastHandleDeath(const FVector& DeathImpulse);

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnHealthChange;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMaxHealthChange;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	UFUNCTION(BlueprintCallable)
	void SetToStunned(bool bIsStunned) const;

	UFUNCTION(BlueprintCallable)
	void SetToEndStun();
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bStunned = false;

	UFUNCTION(BlueprintImplementableEvent)
	void DissolveMesh();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	AActor* CombatTargetCombat;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UREnemyCombatComponent* EnemyCombatComp;
	
protected:
	
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeAttributes() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* HealthBar;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USkeletalMeshComponent* WeaponRH;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	USkeletalMeshComponent* WeaponLH;
	
	//Weapon Damage Socket
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponDamageSocket;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UBehaviorTree* BehaviorTree;

	UPROPERTY()
	ARAIController* AIC;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Combat")
	FText BossName;
};
