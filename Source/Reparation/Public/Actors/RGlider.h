// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Locomotion/RLocomotionTypes.h"
#include "RGlider.generated.h"

class ACharacter;
class URLocomotionComponent;

/**
 * A wing. Carries its own flight characteristics, so a starter glider and a
 * late game one differ by data rather than by code, and an upgrade is a change
 * to these numbers.
 *
 * Owns none of the glide state machine. Locomotion decides when a glide starts
 * and ends; this supplies the handling and the presentation.
 */
UCLASS()
class REPARATION_API ARGlider : public AActor
{
	GENERATED_BODY()

public:

	ARGlider();

	/**
	 * Attaches to the character, pushes this wing's handling onto their movement
	 * component, and grants the unlock tag. Call it from whatever interaction
	 * hands the glider over.
	 */
	UFUNCTION(BlueprintCallable, Category = "Glider")
	void OnAcquired(ACharacter* NewOwner);

	/** Detaches, removes the unlock tag, and stops the character gliding on it. */
	UFUNCTION(BlueprintCallable, Category = "Glider")
	void OnLost();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USkeletalMeshComponent> Mesh;

	/** Handling for this particular wing. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glider")
	FRGlideSettings GlideSettings;

	/**
	 * Granted while carried. Point URLocomotionConfig's GlideUnlockTag at the
	 * same tag and holding this wing is what permits gliding.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glider")
	FGameplayTag UnlockTag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Glider")
	FName AttachSocket = FName("GliderSocket");

protected:

	/** Deploy and stow, for the open animation, wind audio and Niagara. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Glider")
	void OnDeployed();

	UFUNCTION(BlueprintImplementableEvent, Category = "Glider")
	void OnStowed();

	UFUNCTION()
	void HandleLocomotionStateChanged(ERLocomotionState PreviousState, ERLocomotionState NewState);

	UPROPERTY(BlueprintReadOnly, Category = "Glider")
	TObjectPtr<ACharacter> CarriedBy;

	UPROPERTY()
	TObjectPtr<URLocomotionComponent> Locomotion;
};
