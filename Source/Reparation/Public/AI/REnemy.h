// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/RCharacterBase.h"
#include "GAS/Data/RCharacterClassInfo.h"
#include "UI/GAS/Controllers/ROverlayWidgetController.h"
#include "REnemy.generated.h"

class UPawnSensingComponent;
class ARAIController;
class UBehaviorTree;
enum class ECharacterClass : uint8;
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
	virtual int32 GetPLayerLevel_Implementation() override;
	virtual FVector GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag) override;
	virtual void Die() override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;

	virtual void MulticastHandleDeath() override;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnHealthChange;
	
	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangeSignature OnMaxHealthChange;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);
	
	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UFUNCTION(BlueprintImplementableEvent)
	void DissolveMesh();

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	float BaseWalkSpeed = 250.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	AActor* CombatTarget;
	
protected:
	
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeAttributes() const override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UWidgetComponent* HealthBar;

	UPROPERTY(EditAnywhere, Category = "Combat")
	USkeletalMeshComponent* Weapon;

	//Weapon Damage Socket
	UPROPERTY(EditAnywhere, Category = "Combat")
	FName WeaponDamageSocket;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	ECharacterClass CharacterClass = ECharacterClass::Ranger;

	UPROPERTY(EditAnywhere, Category = "AI")
	UBehaviorTree* BehaviorTree;

	UPROPERTY()
	ARAIController* AIC;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UPawnSensingComponent* PawnSensingComp;
	
	
};
