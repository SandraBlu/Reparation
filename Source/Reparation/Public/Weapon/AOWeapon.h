// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AOWeapon.generated.h"

class AAOCharacter;
class USoundBase;
class UBoxComponent;

UCLASS()
class REPARATION_API AAOWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAOWeapon();

//Actors to ignore after weapon hits them (same swing)
	TArray<AActor*> IgnoreActors;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponSocket")
	FName ActiveWeaponSocket;

	UPROPERTY(EditDefaultsOnly, Category = "WeaponSocket")
	FName InactiveWeaponSocket;

	UPROPERTY(EditAnywhere, Category = "FiringSocket")
	FName FiringSocket;

	UPROPERTY(EditDefaultsOnly, Category = "Montages")
	UAnimMontage* EquipAnim;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* EquipSFX;

	UPROPERTY(EditDefaultsOnly, Category = "SFX")
	USoundBase* DisarmSFX;

	/**collision mesh*/
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = Components)
	USkeletalMeshComponent* SKMesh;

	////The weapon item in the players inventory
	//UPROPERTY(BlueprintReadOnly)
	//class UAOWeaponItem* Item;

	UFUNCTION(BlueprintCallable)
	void PlayEquipMontage(const FName SectionName);

	/** weapon is being equipped by owner pawn */
	virtual void OnEquip();

	/** weapon is holstered by owner pawn */
	virtual void OnUnEquip();

	/** attaches weapon mesh to pawn's mesh */
	void AttachMeshToPawn(USceneComponent* InParent, FName InSocketName);

	/** get weapon mesh (needs pawn owner to determine variant) */
	UFUNCTION(BlueprintPure, Category = "Weapon")
	USkeletalMeshComponent* GetWeaponMesh() const;

	UFUNCTION(BlueprintCallable)
	void OnBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	//Public getter for private weapon box
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }

	UFUNCTION(BlueprintImplementableEvent)
	void OnHit(FHitResult BoxHit);


protected:

	virtual void BeginPlay() override;

	//Implement Field System on Weapon
	UFUNCTION(BlueprintImplementableEvent)
	void CreateForceFields(const FVector& FieldLocation);

	/** get pawn owner */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	class AAOCharacter* GetPawnOwner() const;

	UPROPERTY()
	class AAOCharacter* PawnOwner;


private:
	//Weapon Box Settings
	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	UBoxComponent* WeaponBox;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USceneComponent* TraceStart;

	UPROPERTY(VisibleAnywhere, Category = "Weapon")
	USceneComponent* TraceEnd;

};
