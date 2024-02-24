// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AOCharacter.h"
#include "AOPlayerBase.generated.h"

USTRUCT()
struct FInteractData
{
	GENERATED_BODY()

	FInteractData()
	{
		ViewedInteractComp = nullptr;
		LastInteractCheckTime = 0.f;
		bInteractHeld = false;
	}

	UPROPERTY()
	class UAOInteractComponent* ViewedInteractComp;

	UPROPERTY()
	float LastInteractCheckTime;

	UPROPERTY()
	bool bInteractHeld;
};

//DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEquippedItemsChanged, const EEquipSlot, Slot, const UAOEquipItem*, Item);


UCLASS()
class REPARATION_API AAOPlayerBase : public AAOCharacter
{
	GENERATED_BODY()

public:

	AAOPlayerBase();

	//Inventory Comp
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UAOInventoryComponent* PlayerInventory;

	//Add default meshes in case we do not have clothing item
	/*UPROPERTY(BlueprintReadOnly, Category = Mesh)
	TMap<EEquipSlot, USkeletalMesh*>  NakedMeshes;*/
	
	// Equip Map links slots to PlayerMeshes
	/*UPROPERTY(BlueprintReadOnly, Category = Mesh)
	TMap<EEquipSlot, USkeletalMeshComponent*> PlayerMeshes;*/
	
	//Player Meshes
	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* HeadAccessoryMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* TorsoMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* LegsMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* HandsMesh;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class USkeletalMeshComponent* FeetMesh;
	UPROPERTY(EditAnywhere, Category = "Components")
	class USkeletalMeshComponent* BackpackMesh;
	
	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	//UInteractComponent* PlayerLootInteract;

	protected:
	
	virtual void Tick(float DeltaTime) override;
	//virtual void Restart() override;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Components")
	class UCameraComponent* FollowCam;

	/*EEquipState EquipState = EEquipState::EES_Unequipped;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	EActionState ActionState = EActionState::EAS_Idle;*/

	//UFUNCTION(BlueprintCallable)
	//bool CanAttack();

	//UFUNCTION(BlueprintCallable)
	//void AttackEnd();

	//bool CanDisarm();
	//bool CanDraw();
	//
	//UFUNCTION(BlueprintCallable)
	//void Disarm();

	/*UFUNCTION(BlueprintCallable)
	void Draw();*/

	//UFUNCTION(BlueprintCallable)
	//void FinishEquipping();

	////GameplayTags---------------
	//UPROPERTY(EditDefaultsOnly)
	//FGameplayTag AttackEventTag;

	////GameplayTags---------------
	//UPROPERTY(EditDefaultsOnly)
	//FGameplayTag AttackEndEventTag;

	//UPROPERTY()
	//TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

public:

	UPROPERTY(EditAnywhere, Category = "Weapon")
	class AAOWeapon* EquippedWeapon;

	////Interacting---------------------------------------
	UPROPERTY()
	FInteractData InteractData;

	//Helper Function
	FORCEINLINE class UAOInteractComponent* GetInteractable() const { return InteractData.ViewedInteractComp; }

	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float InteractCheckFrequency;

	UPROPERTY(EditDefaultsOnly, Category = "Trace")
	float TraceDistance;
	void PerformInteractCheck();
	void NoInteractableFound();
	void NewInteractableFound(class UAOInteractComponent* Interactable);
	void Interact();
	void OnStartInteract();
	void OnCompleteInteract();

	FTimerHandle TimerHandle_Interact;

 	//-------------------------------------------
 	//Equipment Map holding current Equipped Items
 	/*UPROPERTY(VisibleAnywhere, Category = "Items")
 	TMap<EEquipSlot, UAOEquipItem*> EquippedItems;*/


	//Weapon--------------------
	/*UFUNCTION()
	void OnWeaponEquipped();

	UFUNCTION(BlueprintCallable)
	void SetWeaponCollision(ECollisionEnabled::Type CollisionEnabled);*/

	//called when falling off cliff
	//void UntimelyDeath(struct FDamageEvent const& DamageEvent, const AActor* DamageCauser);

	//Throwable--------------------------------
	UFUNCTION()
	/*void PlayTossFX(class UAnimMontage* TossMontage);
	class UThrowableItem* GetThrowable() const;
	void SpawnThrowable();
	bool CanUseThrowable() const;*/

	////Timed Interact helpers----------
	bool IsInteracting() const;
	float GetRemainingInteractTime() const;

	//Items-------------------
	UFUNCTION(BlueprintCallable, Category = "Items")
	void UseItem(class UAOItem* Item);

	/*UFUNCTION(BlueprintCallable, Category = "Items")
	void DropItem(class UAOItem* Item, const int32 Quantity);*/


 	UPROPERTY(EditDefaultsOnly, Category = "Item")
 	TSubclassOf<class AAOPickup> PickupClass;

 	//Equipment
 	/*bool EquipItem(class UAOEquipItem* Item);
 	bool UnequipItem(class UAOEquipItem* Item);*/
 
 	/*void EquipGear(class UAOGearItem* Gear);
 	void UnequipGear(const EEquipSlot Slot);
 
 	void EquipWeapon(class UAOWeaponItem* WeaponItem);
 	void UnequipWeapon();*/

 	//Equip Delegate
 //	UPROPERTY(BlueprintAssignable, Category = "Item")
 //	FOnEquippedItemsChanged OnEquippedItemsChanged;

 //	//Equipment; BP Access SK Mesh Comp Slot
 //	UFUNCTION(BlueprintPure)
 //	class USkeletalMeshComponent* GetSlotSkeletalMeshComponent(const EEquipSlot Slot);
 //
 //	UFUNCTION(BlueprintPure)
 //	FORCEINLINE TMap<EEquipSlot, UAOEquipItem*> GetEquippedItems() const { return EquippedItems; }

	//UFUNCTION(BlueprintPure)
	//FORCEINLINE class AAOWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

	//UFUNCTION(BlueprintPure)
	//FORCEINLINE EEquipState GetEquipState() const { return EquipState; }

	//UFUNCTION(BlueprintPure)
	//FORCEINLINE EActionState GetActionState() const { return ActionState; }

	void DrawWeapon();
	void EnterAttack();
	void UseThrowable();

	
	
};
