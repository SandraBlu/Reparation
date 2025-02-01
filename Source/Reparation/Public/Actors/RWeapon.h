// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "RWeapon.generated.h"

class UNiagaraComponent;
class UBoxComponent;
class UPawnCombatComponent;

UENUM(BlueprintType)
enum class ECombatType : UINT8
{
	ECT_None UMETA(DisplayName = "None"),
	ECT_LightSword UMETA(DisplayName = "Sword"),
	ECT_Staff UMETA(DisplayName = "Staff"),
	ECT_Ranged UMETA(DisplayName = "Ranged"),
	ECT_Throwable UMETA(DisplayName = "Throwable")
};

UCLASS()
class REPARATION_API ARWeapon : public AActor
{
	GENERATED_BODY()
	
public:	

	ARWeapon();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	ECombatType CombatType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	FGameplayTag WeaponTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	FName ActiveWeaponSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	FName InactiveWeaponSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	FName FiringSocket;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Settings")
	UAnimMontage* DisarmAnim;
	
	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	USoundBase* EquipSFX;

	UPROPERTY(EditDefaultsOnly, Category = "Settings")
	USoundBase* DisarmSFX;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "FX")
	UNiagaraComponent* TrailVFX;

	/**collision mesh*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	USkeletalMeshComponent* SKMesh;

	UFUNCTION(BlueprintCallable)
	void PlayEquipMontage(const FName SectionName);

	/** weapon is being equipped by owner pawn */
	UFUNCTION(BlueprintCallable)
	virtual void OnEquip();

	/** weapon is holstered by owner pawn */
	UFUNCTION(BlueprintCallable)
	virtual void OnUnEquip();

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn(USceneComponent* InParent, FName InSocketName);

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;
	
	FORCEINLINE FGameplayTag GetWeaponTag() const { return WeaponTag; }

	
protected:

	virtual void BeginPlay() override;
	
	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class ARPlayer* GetPawnOwner() const;

	UPROPERTY()
	class ARPlayer* PawnOwner;
	

	
};

