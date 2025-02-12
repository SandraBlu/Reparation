// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GenericTeamAgentInterface.h"
#include "GameFramework/PlayerController.h"
#include "RPlayerController.generated.h"

class ATargetCircle;
class UDamageTextComponent;
/**
 * 
 */
UCLASS()
class REPARATION_API ARPlayerController : public APlayerController, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:

	ARPlayerController();

	virtual FGenericTeamId GetGenericTeamId() const override;
	
	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bDodgedHit, bool bCriticalHit);
	
private:

	FGenericTeamId PlayerTeamID;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;
	
};
