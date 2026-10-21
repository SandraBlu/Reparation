// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/RCharacterBase.h"
#include "RCompanion.generated.h"

class UTextRenderComponent;
class URCompanionAttackAbility;
class URCompanionPartyComponent;
class USkeletalMeshComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API ARCompanion : public ARCharacterBase
{
	GENERATED_BODY()

public:
	
	ARCompanion(const FObjectInitializer& ObjectInitializer);
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	virtual void Die(const FVector& DeathImpulse) override;
	bool CanAttackTarget(AActor* Target) const;
	bool IsAttackActive() const;
	bool TryAttack();
	void CancelAttack();
	UFUNCTION(BlueprintPure, Category="Companion|Combat")
	bool IsCombatReady() const;
	UFUNCTION(BlueprintPure, Category="Companion|Combat")
	FText GetCombatReadinessIssue() const;
	/** Assign a weapon mesh to either component, or both for dual blades. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Companion|Weapons")
	TObjectPtr<USkeletalMeshComponent> RightWeaponSlot;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Companion|Weapons")
	TObjectPtr<USkeletalMeshComponent> LeftWeaponSlot;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Weapons")
	FName RightWeaponSocket = TEXT("weapon_r");
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Weapons")
	FName LeftWeaponSocket = TEXT("weapon_l");
	/** Disable for companions whose attacks need no held weapon. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Weapons")
	bool bRequiresEquippedWeapon = false;
	UFUNCTION(BlueprintPure, Category="Companion|Weapons")
	bool HasEquippedWeapon() const;
	UFUNCTION(BlueprintPure, Category="Companion|Weapons")
	bool HasAssignedWeapon() const;
	UFUNCTION(BlueprintCallable, Category="Companion|Weapons")
	bool EquipAssignedWeapons();
	UFUNCTION(BlueprintCallable, Category="Companion|Weapons")
	void RefreshWeaponAttachments();
	virtual void OnConstruction(const FTransform& Transform) override;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Assistance")
	bool bAutomaticAssistance = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Assistance", meta=(ClampMin="0"))
	float AssistanceRadius = 1500.f;
	/** Continue combat through a brief pause in the enemy's targeting. Death ends it immediately. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Assistance", meta=(ClampMin="0"))
	float AssistanceDisengageDelay = 3.f;
	TWeakObjectPtr<URCompanionPartyComponent> RegisteredParty;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Companion|Combat", meta=(ClampMin="1"))
	int32 CompanionLevel = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Companion|Combat")
	TSubclassOf<URCompanionAttackAbility> AttackAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Companion|Signature Skill") TSubclassOf<UGameplayAbility> SignatureAbility;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Companion|Signature Skill") FText SignatureSkillName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Companion|Signature Skill", meta=(ClampMin="1")) float SignatureCooldown = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion")
	FText CompanionDisplayName;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Companion|UI")
	TObjectPtr<UTextRenderComponent> SelectionMarker;
protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeAttributes() const override;
private:
	double NextSignatureTime = 0.0;
	UPROPERTY(Transient) TObjectPtr<AActor> CombatTarget;
	bool bAttributesInitialized = false;
};
