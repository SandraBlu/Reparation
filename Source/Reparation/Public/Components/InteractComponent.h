// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFoundReactor,class  UReactionComponent*, React );
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLostReactor, class  UReactionComponent*, Reactor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteracted, class  UInteractComponent*, Interact, class  UReactionComponent*, Reactor);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractPressed, class  UInteractComponent*, Interact);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteractReleased, class  UInteractComponent*, Interact);

UCLASS( ClassGroup=(Reparation), DisplayName = "Game Interaction", meta=(BlueprintSpawnableComponent) )
class REPARATION_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	

//[local + server] Called when we find a new interactable object 
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable, Category = "Interact")
	FOnFoundReactor OnFoundReaction;

	//[local + server] Called when we've lost our interactable
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable, Category = "Interact")
	FOnLostReactor OnLostReaction;

	//[local + server] Called when we interact with an interactable 
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable, Category = "Interact")
	FOnInteracted OnInteracted;

	//[local + server] Called when we start holding the interact key 
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable, Category = "Interact")
	FOnInteractPressed OnInteractPressed;

	//[local + server] Called when we release the interact key 
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable, Category = "Interact")
	FOnInteractReleased OnInteractReleased;

protected:

	UInteractComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void Deactivate() override;

	virtual void PerformInteractCheck(float DeltaTime);

	void NoReactorFound();
	void FoundReactor(UReactionComponent* Reactor);

	UFUNCTION(Server, Reliable)
	void ServerBeginInteract();

	UFUNCTION(Server, Reliable)
	void ServerEndInteract();

	UFUNCTION(BlueprintCallable, Category = "Interact")
	virtual void BeginInteract();

	UFUNCTION(BlueprintCallable, Category = "Interact")
	virtual void EndInteract();

protected:

	//Our pawn owner
	UPROPERTY(BlueprintReadOnly, Category = "Interact")
	class APawn* OwningPawn;

	//Our controller owner
	UPROPERTY(BlueprintReadOnly, Category = "Interact")
	class APlayerController* OwningController;

	//The current interactable component we're viewing, if there is one
	UPROPERTY(BlueprintReadOnly, Category = "Interact")
	class UReactionComponent* CurrentReactor;

	//The time when we last checked for an interactable
	UPROPERTY()
	float LastInteractCheckTime;

	//Whether the local player is holding the interact key
	UPROPERTY()
	bool bInteractHeld;

	/**The amount of time before interaction completes on our current interactable.This is stored per interactor and not per interactable
	because in a networked game we may want to support multiple players interacting with something and each will have their own time*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interact")
	float RemainingInteractTime;

	//How often in seconds to check for an interactable object. Set this to zero if you want to check every tick.
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float InteractCheckFrequency;

	//How far we'll trace when we check if the player is looking at an interactable object
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float InteractCheckDistance;

	//If greater than zero we'll use a sphere trace over an interaction trace 
	UPROPERTY(EditDefaultsOnly, Category = "Interact")
	float InteractCheckSphereRadius;

		
};
