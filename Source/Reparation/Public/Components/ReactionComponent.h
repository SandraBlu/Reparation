// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReactionComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBeginInteract, class APawn*, Interactor, class UInteractComponent*, InteractComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEndInteract, class APawn*, Interactor, class UInteractComponent*, InteractComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBeginFocus, class APawn*, Interactor, class UInteractComponent*, InteractComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEndFocus, class APawn*, Interactor, class UInteractComponent*, InteractComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteract, class APawn*, Interactor, class UInteractComponent*, InteractComp);

UCLASS( ClassGroup=(Reparation), Blueprintable, DisplayName = "Reparation Reactor", meta=(BlueprintSpawnableComponent) )
class REPARATION_API UReactionComponent : public UActorComponent
{
	GENERATED_BODY()

protected:

	friend class UInteractComponent;

public:	
	// Sets default values for this component's properties
	UReactionComponent();

	virtual void Deactivate() override;
	void SetupAttachment(class UStaticMeshComponent* PickupMesh);

	//The time the player must hold the interact key to interact with this object
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact", meta = (ClampMin=0))
	float InteractTime;

	//The max distance the player can be away from this actor before you can interact
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float InteractDistance;

	//The name that will come up when the player looks at the interactable
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	FText ReactorItemName;

	//The verb that describes how the interaction works, ie "Sit" for a chair, "Eat" for food, "Light" for a fireplace
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	FText ReactorActionText;

	//A material to draw over any mesh components our owner has when this interatable is focused 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	class UMaterialInterface* FocusedOverlayMaterial;

public:

	//Call this to change the name of the interactable. Will also refresh the interaction widget.
	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetReactorNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetReactorActionText(const FText& NewActionText);

protected:

	//Called on the client when the players interaction check trace begins/ends hitting this item
	virtual void BeginFocus(class APawn* Interactor, class UInteractComponent* InteractComp);
	virtual void EndFocus(class APawn* Interactor, class UInteractComponent* InteractComp);

	//Called on the client when the player begins/ends/interacts with the item
	virtual void BeginInteract(class APawn* Interactor, class UInteractComponent* InteractComp);
	virtual void EndInteract(class APawn* Interactor, class UInteractComponent* InteractComp);
	virtual bool Interact(class APawn* Interactor, class UInteractComponent* InteractComp);

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void OnInteract(class APawn* Interactor, class UInteractComponent* InteractComp);
	virtual void OnInteract_Implementation(class APawn* Interactor, class UInteractComponent* InteractComp);

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void OnBeginInteract(class APawn* Interactor, class UInteractComponent* InteractionComp);
	virtual void OnBeginInteract_Implementation(class APawn* Interactor, class UInteractComponent* InteractComp);

	UFUNCTION(BlueprintNativeEvent, Category = "Item")
	void OnEndInteract(class APawn* Interactor, class UInteractComponent* InteractionComp);
	virtual void OnEndInteract_Implementation(class APawn* Interactor, class UInteractComponent* InteractComp);

	/**
	* Return true if the given interactor is allowed to interact with us
	* 
	* @param OutErrorText this is the reason why the interact isn't allowed 
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Item")
	bool CanInteract(class APawn* Interactor, class UInteractComponent* InteractComp, FText& OutErrorText);
	virtual bool CanInteract_Implementation(class APawn* Interactor, class UInteractComponent* InteractComp, FText& OutErrorText);

protected:

	//Delegates

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(BlueprintAssignable)
	FOnBeginInteract OnBeginInteracted;

	//[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact
	UPROPERTY(BlueprintAssignable)
	FOnEndInteract OnEndInteracted;

	//[local + server] Called when the player presses the interact key whilst focusing on this interactable actor
	UPROPERTY(BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;

	//[local + server] Called when the player releases the interact key, stops looking at the interactable actor, or gets too far away after starting an interact
	UPROPERTY(BlueprintAssignable)
	FOnEndFocus OnEndFocus;

	//[local + server] Called when the player has interacted with the item for the required amount of time
	UPROPERTY(BlueprintAssignable)
	FOnInteract OnInteracted;

		
};
