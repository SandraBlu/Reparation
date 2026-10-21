// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "RAIController.generated.h"

struct FAIStimulus;
/**
 * 
 */
UCLASS()
class REPARATION_API ARAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	
	ARAIController(const FObjectInitializer& ObjectInitializer);

	virtual  ETeamAttitude::Type GetTeamAttitudeTowards(const AActor& Other) const override;
	virtual void Tick(float DeltaSeconds) override;
	void HandleSightUpdate(AActor* Actor, bool bVisible, const FVector& SeenLocation);
	bool IsWithinSightCone(const FVector& Location) const;
	AActor* GetSightTarget() const { return SightTarget.Get(); }
	bool UsesBehaviorTreeSearch() const;
	void EnableBehaviorTreeSearch() { bBehaviorTreeSearchEnabled = true; }
	void StartSearchMovement();
	void TickSearchMovement(float DeltaSeconds);
	void NotifyDamagedBy(AActor* Attacker, float Damage);
	/** Angle to each side of the pawn's forward direction. 60 means a 120-degree cone. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Sight", meta=(ClampMin="0", ClampMax="180"))
	float SightHalfAngle = 60.f;

	UPROPERTY(BlueprintReadOnly, Category="AI|Search")
	bool bSearching = false;
	UPROPERTY(BlueprintReadOnly, Category="AI|Search")
	FVector LastKnownLocation = FVector::ZeroVector;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Search", meta=(ClampMin="0"))
	float SearchRadius = 600.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Search", meta=(ClampMin="0.5"))
	float SearchStepInterval = 4.f;
	/** Rotate while examining each search point instead of waiting with our back turned. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="AI|Search", meta=(ClampMin="1"))
	float SearchScanTurnRate = 90.f;
	
protected:

	virtual void BeginPlay() override;

	UPROPERTY()
	UBehaviorTree* BTree;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior Tree")
	class UAIPerceptionComponent* PerceptionComp;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior Tree")
	class UAISenseConfig_Sight* AISightConfig;

	UFUNCTION()
	virtual void OnEnemyPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

private:
	struct FThreatEntry { TWeakObjectPtr<AActor> Actor; float Score = 0.f; };
	TArray<FThreatEntry> Threats;
	float TargetCommitmentRemaining = 0.f;
	TWeakObjectPtr<AActor> PendingDamageSource;
	void FaceDamageSource(AActor* Attacker);
	void UpdateThreats(const TArray<AActor*>& VisibleActors, float DeltaSeconds);
	void SelectCombatTarget(AActor* Actor, const FVector& SeenLocation);
	bool bBehaviorTreeSearchEnabled = false;
	TWeakObjectPtr<AActor> SightTarget;
	float SearchStepTime = 0.f;
	bool bSearchWasMoving = false;
	float SearchMoveElapsed = 0.f;
	float SearchBlockedTime = 0.f;
	bool bHasSeenTarget = false;
	void BeginSearch();
	void ClearCombatTarget();
	UPROPERTY(EditDefaultsOnly, Category = "Crowd Avoidance Config")
	bool bEnableCrowdAvoidance = true;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowd Avoidance Config", meta = (EditCondition = "bEnableCrowdAvoidance",UIMin = "1",UIMax = "4"))
	int32 CrowdAvoidanceQuality = 4;
	
	UPROPERTY(EditDefaultsOnly, Category = "Crowd Avoidance Config", meta = (EditCondition = "bEnableCrowdAvoidance"))
	float CollisionQueryRange = 600.f;
	
};
