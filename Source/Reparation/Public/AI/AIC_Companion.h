// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "AIC_Companion.generated.h"

class ATargetPoint;

UENUM(BlueprintType)
enum class ERCompanionOrder : uint8 { Stay, Follow, GoToLocation, Attack };

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FRCompanionOrderResult, ERCompanionOrder, Order, bool, bSucceeded);

/** C++ ground movement executor. No Behavior Tree required. */
UCLASS()
class REPARATION_API AAIC_Companion : public AAIController
{
	GENERATED_BODY()
	
public:
	
	AAIC_Companion(const FObjectInitializer& ObjectInitializer);
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	UFUNCTION(BlueprintCallable, Category="Companion|Commands")
	bool FollowLeader(ACharacter* Leader);
	UFUNCTION(BlueprintCallable, Category="Companion|Commands")
	void Stay();
	UFUNCTION(BlueprintCallable, Category="Companion|Commands")
	bool GoToLocation(FVector Location);
	UFUNCTION(BlueprintCallable, Category="Companion|Commands")
	bool Attack(AActor* Target);
	bool TryAutomaticAssist(AActor* Target, ACharacter* Leader);
	UPROPERTY(BlueprintReadOnly, Category="Companion|Assistance")
	bool bAssistingAutomatically = false;

	UPROPERTY(BlueprintReadOnly, Category="Companion|Commands")
	ERCompanionOrder CurrentOrder = ERCompanionOrder::Stay;
	UPROPERTY(BlueprintAssignable, Category="Companion|Commands")
	FRCompanionOrderResult OnOrderFinished;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Movement", meta=(ClampMin="1"))
	float ArrivalDistance = 75.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Movement", meta=(ClampMin="0"))
	float SneakMirrorDistance = 750.f;
	/** Offset in the leader's local space; use different offsets for each party member. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Movement")
	FVector FormationOffset = FVector(-250.f, 0.f, 0.f);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Stealth")
	bool bMirrorPlayerSneaking = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Combat", meta=(ClampMin="1"))
	float MaxChaseDistance = 5000.f;
	/** Allows cooldowns, but reports failure if an attack cannot activate for this long. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Combat", meta=(ClampMin="1"))
	float AttackActivationTimeout = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Companion|Combat", meta=(ClampMin="0.1"))
	float AttackInterval = 0.75f;
protected:
	virtual void OnUnPossess() override;

private:
	TWeakObjectPtr<ACharacter> AssistanceLeader;
	float AssistanceRetryDelay = 0.f;
	float AssistanceQuietTime = 0.f;
	bool bDefendingSelf = false;
	UPROPERTY()
	TObjectPtr<ACharacter> FollowTarget;
	FAIRequestID ActiveMove;
	bool bAppliedLeaderCrouch = false;
	void CancelCurrentMove();
	void CancelCombat();
	void TickCombat(float DeltaSeconds);
	void FinishCombat(bool bSucceeded);
	void RestoreFollowSpeed();
	float NormalWalkSpeed = 0.f;
	float NormalCrouchSpeed = 0.f;
	bool bFollowSpeedSaved = false;
	bool bNormalPathAcceleration = false;
	TWeakObjectPtr<ATargetPoint> FollowGoalActor;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UPROPERTY(Transient) TObjectPtr<AActor> AttackTarget;
	ERCompanionOrder ResumeOrder = ERCompanionOrder::Stay;
	FVector OrderDestination = FVector::ZeroVector;
	FVector ResumeDestination = FVector::ZeroVector;
	FVector CombatOrigin = FVector::ZeroVector;
	float AttackWait = 0.f;
	float AttackDelay = 0.f;
	bool bReturningToStay = false;
	
};
