// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "RAnimInstanceBase.h"
#include "Actors/RWeapon.h"
#include "RAnimPlayer.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API URAnimPlayer : public URAnimInstanceBase
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|References")
	class ARPlayer* OwningPlayerCharacter;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|References")
	bool bEnterRelaxState;

	/**
	 * Weapon overlay the graph blends by. Reads ECT_None when nothing is
	 * equipped, so an unarmed pin is always valid.
	 */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "AnimData|Combat")
	ECombatType CombatType = ECombatType::ECT_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AnimData|References")
	float EnterRelaxStateThreshold = 5.f;
	
	float IdleElapsedTime;
	
};
