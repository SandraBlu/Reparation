// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "AOInteractComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginInteract, class AAOPlayerBase*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndInteract, class AAOPlayerBase*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBeginFocus, class AAOPlayerBase*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEndFocus, class AAOPlayerBase*, Character);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInteract, class AAOPlayerBase*, Character);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class REPARATION_API UAOInteractComponent : public UWidgetComponent
{
	GENERATED_BODY()

public:

	UAOInteractComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float InteractTime;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	float InteractDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	FText InteractItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interact")
	FText InteractActionText;

	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetInteractNameText(const FText& NewNameText);

	UFUNCTION(BlueprintCallable, Category = "Interact")
	void SetInteractActionText(const FText& NewActionText);

	//Delegates
	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginFocus OnBeginFocus;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndFocus OnEndFocus;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnBeginInteract;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnEndInteract OnEndInteract;

	UPROPERTY(EditDefaultsOnly, BlueprintAssignable)
	FOnBeginInteract OnInteract;

protected:

	virtual void Deactivate() override;
	bool CanInteract(class AAOPlayerBase* Character) const;

	UPROPERTY()
	TArray<class AAOPlayerBase*> Interactors;

public:

	void RefreshWidget();

	void BeginFocus(class AAOPlayerBase* Character);
	void EndFocus(class AAOPlayerBase* Character);

	void BeginInteract(class AAOPlayerBase* Character);
	void EndInteract(class AAOPlayerBase* Character);
	void Interact(class AAOPlayerBase* Character);

	UFUNCTION(BlueprintPure, Category = "Interact")
	float GetInteractPercentage(class AAOPlayerBase* Character);
	
};
