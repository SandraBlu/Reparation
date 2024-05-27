// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "RWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangeSignature, int32, NewValue);

class UAbilitySystemComponent;
class UAttributeSet;
class ARPlayerController;
class ARPlayerState;
class URAbilitySystemComponent;
class URAttributeSet;
class URAbilityInfo;

USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY()

	FWidgetControllerParams() {}
	FWidgetControllerParams(APlayerController* PC, APlayerState* PS, UAbilitySystemComponent* ASC, UAttributeSet* AS) 
	: PlayerController(PC), PlayerState(PS), AbilitySystemComponent(ASC), AttributeSet(AS) {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APlayerController* PlayerController = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	APlayerState* PlayerState = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAbilitySystemComponent* AbilitySystemComponent = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAttributeSet* AttributeSet = nullptr;
};
/**
 * 
 */
UCLASS()
class REPARATION_API URWidgetController : public UObject
{
	GENERATED_BODY()

	public:

	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WCParams);

	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();
	virtual void BindCallbacksToDependencies();
	void BroadcastAbilityInfo();
	
protected:

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	APlayerController* PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	APlayerState* PlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	UAttributeSet* AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	ARPlayerController* RPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	ARPlayerState* RPlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	URAbilitySystemComponent* RAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	URAttributeSet* RAttributeSet;

	ARPlayerController* GetRPC();
	ARPlayerState* GetRPS();
	URAbilitySystemComponent* GetRASC();
	URAttributeSet* GetRAS();
	
};
