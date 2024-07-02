// Copyright 2023 Klian. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ReplicatedTossComponent.generated.h"

#pragma region Declarations

	/*================================================================== Structs ==================================================================*/

UENUM(BlueprintType)
enum class EReplicatedTossPrevisionValidity : uint8 {
	PV_None			UMETA(DisplayName = "None"),
	PV_True			UMETA(DisplayName = "True"),
	PV_False		UMETA(DisplayName = "False"),
};

	/*================================================================== Structs ==================================================================*/

USTRUCT(BlueprintType)
struct REPLICATEDTOSS_API FReplicatedTossPrevisionParameters
{
	GENERATED_USTRUCT_BODY()

	/*Max toss force when using Arch prevision*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		float MaxArchForce;
	/*Max toss force when using Linear prevision*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		float MaxLinearForce;
	/*Niagara system to use for this particular prevision*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		class UNiagaraSystem* TossAimTrail;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		uint8 bTraceWithCollision : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		uint8 bTraceComplex : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		float ProjectileRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		float OverrideGravityZ;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss", meta = (EditCondition = "!bTraceWithChannel", EditConditionHides))
		TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		uint8 bTraceWithChannel : 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss", meta = (EditCondition = "bTraceWithChannel", EditConditionHides))
		TEnumAsByte<ECollisionChannel> TraceChannel;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		float MaxSimTime;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		float SimFrequency;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		uint8 DrawDebug : 1;

	
	// Default initialization
	FReplicatedTossPrevisionParameters()
	{
		MaxArchForce = 1000.0f;
		MaxLinearForce = 2000.0f;
		TossAimTrail = nullptr;

		bTraceWithCollision = true;
		bTraceComplex = false;
		ProjectileRadius = 4.0f;
		OverrideGravityZ = 0.0f;
		ObjectTypes = TArray<TEnumAsByte<EObjectTypeQuery>>{ EObjectTypeQuery ::ObjectTypeQuery1};
		bTraceWithChannel = true;
		TraceChannel = ECollisionChannel::ECC_Visibility;
		MaxSimTime = 15.0f;
		SimFrequency = 10.0f;

		DrawDebug = false;
	}

	// Initialization with parameters
	FReplicatedTossPrevisionParameters(const float InMaxArch,
		const float InMaxLinear,
		class UNiagaraSystem* InNiagaraSystem,
		const bool InTraceWithCollision,
		const bool InTraceComplex,
		const float InProjectileRadius,
		const float InOverrideGravityZ,
		const TArray<TEnumAsByte<EObjectTypeQuery>> InObjectTypes,
		const bool InTraceWithChannel,
		const TEnumAsByte<ECollisionChannel> InCollisionChannel,
		const float InMaxSimTime,
		const float InSimFrequency,
		const bool InDrawDebug = false)
	{
		MaxArchForce = InMaxArch;
		MaxLinearForce = InMaxLinear;
		TossAimTrail = InNiagaraSystem;

		bTraceWithCollision = InTraceWithCollision;
		bTraceComplex = InTraceComplex;
		ProjectileRadius = InProjectileRadius;
		OverrideGravityZ = InOverrideGravityZ;
		ObjectTypes = InObjectTypes;
		bTraceWithChannel = InTraceWithChannel;
		TraceChannel = InCollisionChannel;
		MaxSimTime = InMaxSimTime;
		SimFrequency = InSimFrequency;

		DrawDebug = InDrawDebug;
	}
};

USTRUCT(BlueprintType)
struct REPLICATEDTOSS_API FTossOriginInformation
{
	GENERATED_USTRUCT_BODY()

	/*Toss origin component to use for both prevision and toss action*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		USceneComponent* Origin;
	/*Origin's socket to use, if empty the component's origin will be used instead*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ReplicatedToss")
		FName SocketName;
	
	// Default initialization
	FTossOriginInformation()
	{
		Origin = nullptr;
		SocketName = FName("");
	}

	// Initialization with component only
	FTossOriginInformation(USceneComponent* InOrigin)
	{
		Origin = InOrigin;
		SocketName = FName("");
	}

	// Initialization with both component and socket
	FTossOriginInformation(USceneComponent* InOrigin, const FName InSocketName)
	{
		Origin = InOrigin;
		SocketName = InSocketName;
	}

	// Function to check if the component is valid
	bool IsValid() {
		return (Origin)? true : false;
	}

	// Function to check if has any socket name (doesn't check if the socket exist)
	bool HasSocketParameter() {
		return SocketName != FName("");
	}
};

	/*================================================================== Delegates ==================================================================*/

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPrevisionValidityChange, EReplicatedTossPrevisionValidity, IsValid);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReplicatedTossActive, bool, IsActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorTossed, AActor*, Actor, UPrimitiveComponent*, Component, FVector, TossForce);

#pragma endregion

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPLICATEDTOSS_API UReplicatedTossComponent : public UActorComponent
{
	GENERATED_BODY()

	/*================================================================== Defaults ==================================================================*/

#pragma region Defaults

public:	
	// Sets default values for this component's properties
	UReplicatedTossComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;	
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

	/*Replicated event, both server and clients, when the component's been activated or deactivated*/
	UPROPERTY(BlueprintAssignable, Category = "ReplicatedToss")
		FOnReplicatedTossActive OnReplicatedTossActive;
	/*Replicated event, both server and clients, when the current prevision changes its validity*/
	UPROPERTY(BlueprintAssignable, Category = "ReplicatedToss")
		FOnPrevisionValidityChange OnPrevisionValidityChange;
	/*Server event, when and actor has been tossed. Happens on server assuming the tossed actor is set as both "bReplicates" and "bReplicateMovement"*/
	UPROPERTY(BlueprintAssignable, Category = "ReplicatedToss")
		FOnActorTossed OnActorTossed;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing = OnRep_ReplicatedTossEnabled)
		uint8 ReplicatedTossEnabled : 1;
	UPROPERTY()
		uint8 bCurrentIsFirstTrace : 1;

	UFUNCTION()
		virtual void OnRep_ReplicatedTossEnabled();

	UFUNCTION(Server, Reliable)
	virtual void ActivateOnServer(bool bReset = false);
	UFUNCTION(Server, Reliable)
	virtual void DeactivateOnServer();	

#pragma endregion

	/*================================================================== Runtime ==================================================================*/

#pragma region Runtime

protected:

	UPROPERTY()
		FVector LastValidPrevisionForce;
	UPROPERTY(ReplicatedUsing = OnRep_PrevisionIsValidCurrentlyChanged)
		EReplicatedTossPrevisionValidity bPrevisionIsValidCurrently;

	UFUNCTION()
		virtual void OnRep_PrevisionIsValidCurrentlyChanged(const EReplicatedTossPrevisionValidity OldValue);
	UFUNCTION()
		virtual void WaitArch_PrevisionIsValidCurrentlyChanged();

	UFUNCTION(Server, Reliable)
		virtual void SetPrevisionIsValidCurrentlyOnServer(const bool IsValid);

public:
	/*Return the last valid prevision force, even if the current prevision is invalid, it will return the last valid one!*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss")
		virtual FVector GetLastValidPrevisionForce() const {
		return LastValidPrevisionForce;
	}
	/*Return true if the prevision function called in the current frame has succeed*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss")
		virtual bool PrevisionIsValidCurrently() const {
		return bPrevisionIsValidCurrently == EReplicatedTossPrevisionValidity::PV_True;
	}

#pragma endregion

	/*================================================================== Editables ==================================================================*/

#pragma region Editables

protected:
	UPROPERTY()
		uint8 bIsUsingArch : 1;

public:	
	/*This component should enable physics to the target actor, if it's not simulating when tossed?*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ReplicatedToss")
		uint8 bAlwaysEnablePhysics : 1;
	/*Should replicate prevision arch visibility to everybody in the current session?*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ReplicatedToss")
		uint8 bReplicatesPrevision : 1;

	/*Return true if the component is using an arch prevision*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss")
		virtual bool IsUsingHighArch() const {
		return bIsUsingArch;
	};
	/*Set what this system should use as prevision trajectory, linear or arch*/
	UFUNCTION(BlueprintCallable, Category = "ReplicatedToss")
		virtual void UseHighArch(const bool Enabled);

#pragma endregion

	/*================================================================== TossOrigin ==================================================================*/

#pragma region TossOrigin

protected:
	UPROPERTY(Replicated)
		FTossOriginInformation TossOrigin;

	UFUNCTION(Server, Reliable)
		void SetTossOrigin_Server(FTossOriginInformation NewTossOrigin);
	UFUNCTION()
		void SetTossOrigin_Func(FTossOriginInformation NewTossOrigin);

public:
	/*Return the origin component if it's been registered*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOrigin"))
		USceneComponent* GetTossOrigin() const {
		return TossOrigin.Origin;
	};
	/*Return the component's socket if set*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginSocket"))
		FName GetTossOriginSocket() const {
		return TossOrigin.SocketName;
	};

	/*Set a new TossOrigin, this is required!*/
	UFUNCTION(BlueprintCallable, Category = "ReplicatedToss")
		void SetTossOrigin(FTossOriginInformation NewTossOrigin);

	/*Return component's transform (World)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginTransform"))
		FTransform GetTossOriginTransform() const;
	/*Return component's location (World)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginLocation"))
		FVector GetTossOriginLocation() const;
	/*Return component's rotation (World)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginRotation"))
		FRotator GetTossOriginRotation() const;
	/*Return component's quat (World)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginQuat"))
		FQuat GetTossOriginQuat() const;

	/*Return component's relative transform (Actor)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginRelativeTransform"))
		FTransform GetTossOriginRelativeTransform() const;
	/*Return component's relative location (Actor)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginRelativeLocation"))
		FVector GetTossOriginRelativeLocation() const;
	/*Return component's relative rotation (Actor)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginRelativeRotation"))
		FRotator GetTossOriginRelativeRotation() const;
	/*Return component's relative quat (Actor)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginRelativeQuat"))
		FQuat GetTossOriginRelativeQuat() const;

	/*Return component's forward vector (World)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginForwardVector"))
		FVector GetTossOriginForwardVector() const {

		return GetTossOriginQuat().GetForwardVector();
	};
	/*Return component's right vector (World)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginRightVector"))
		FVector GetTossOriginRightVector() const {

		return GetTossOriginQuat().GetRightVector();
	};
	/*Return component's up vector (World)*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss", meta = (CompactNodeTitle = "GetOriginUpVector"))
		FVector GetTossOriginUpVector() const {

		return GetTossOriginQuat().GetUpVector();
	};

#pragma endregion

	/*================================================================== TossPrevision ==================================================================*/

#pragma region TossPrevision

public:
	/*[Tick Function] Used for prevision, should be set on tick without any check since it's already optimized for replication*/
	UFUNCTION(BlueprintCallable, Category = "ReplicatedToss")
		bool TracePrevisionTrajectory(const FReplicatedTossPrevisionParameters PrevisionParams, const FVector SimpleDirection, const bool bUseGoalLocation, const FVector GoalLocation, FVector& TossForceResult, const float ForceMulti = 1.0f);

#pragma endregion

	/*================================================================== TossAction ==================================================================*/

#pragma region TossAction
protected:
	UFUNCTION(Server, Reliable)
		void TossActor_Server(AActor* TargetComponent, const FVector TossForce, const float ForceMulti);
	UFUNCTION()
		void TossActor_Func(AActor* TargetComponent, const FVector TossForce, const float ForceMulti);

	UFUNCTION(Server, Reliable)
		void SpawnAndTossActor_Server(TSubclassOf<AActor> ActorClass, const FVector TossForce, const float ForceMulti);
	UFUNCTION()
		void SpawnAndTossActor_Func(TSubclassOf<AActor> ActorClass, const FVector TossForce, const float ForceMulti);

public:
	/*Toss an existing, specified actor. The actor must have both "bReplicates" and "ReplicateMovement" enabled in order to correctly replicate physics*/
	UFUNCTION(BlueprintCallable, Category = "ReplicatedToss")
		void TossActor(AActor* TargetActor, const FVector TossForce, const float ForceMulti = 1.0f);
	/*Spawn and Toss a new actor from specified actor. The class must have both "bReplicates" and "ReplicateMovement" enabled in order to correctly replicate physics*/
	UFUNCTION(BlueprintCallable, Category = "ReplicatedToss")
		void SpawnAndTossActor(TSubclassOf<AActor> ActorClass, const FVector TossForce, const float ForceMulti = 1.0f);

#pragma endregion

	/*================================================================== Visualization ==================================================================*/

#pragma region Visualization

protected:
	UPROPERTY()
		class UNiagaraComponent* CurrentTossAimTrailComponent;	

	UFUNCTION(Server, Unreliable)
		virtual void UpdateArchOnServer(const TArray<FVector>& Points);
public:
	/*Collection of points generated by the prevision trace, by default these poits are used by the niagara system for visualization. This variable is replicated for each player and its updating function is unreliable over the net for clients to avoid overloads, since it's updated each tick*/
	UPROPERTY(BlueprintReadWrite, Category = "ReplicatedToss", Replicated)
		TArray<FVector> ArchOutPoints = TArray<FVector>();
	/*Return a pointer to current Aim Trail Niagara System Instance*/
	UFUNCTION(BlueprintPure, Category = "ReplicatedToss")
		virtual class UNiagaraComponent* GetAimTrail() const;

#pragma endregion
		
};
