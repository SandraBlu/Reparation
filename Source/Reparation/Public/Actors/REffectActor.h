// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayEffectTypes.h"
#include "REffectActor.generated.h"

class UGameplayEffect;
class UAbilitySystemComponent;
class UStaticMeshComponent;

UENUM(BlueprintType)
enum class EApplyEffectPolicy
{
	ApplyOnOverlap,
	ApplyOnEndOverlap,
	DoNotApply
};

UENUM(BlueprintType)
enum class ERemoveEffectPolicy
{
	RemoveOnEndOverlap,
	DoNotRemove
};

USTRUCT(BlueprintType)
struct FEffectType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects")
	TSubclassOf<UGameplayEffect> GameplayEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects")
	EApplyEffectPolicy ApplicationPolicy = EApplyEffectPolicy::DoNotApply;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Effects")
	ERemoveEffectPolicy RemovalPolicy = ERemoveEffectPolicy::DoNotRemove;
};

UCLASS()
class REPARATION_API AREffectActor : public AActor
{
	GENERATED_BODY()
	
public:	

	AREffectActor();

	UFUNCTION(BlueprintCallable)
	void ApplyEffectToTarget(AActor* TargetActor, const FEffectType& Effect);
	
	UFUNCTION(BlueprintCallable)
	void OnOverlap(AActor* TargetActor);
	
	UFUNCTION(BlueprintCallable)
	void OnEndOverlap(AActor* TargetActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	bool bDestroyOnEffectApplication = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	bool bApplyEffectToEnemy = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	EApplyEffectPolicy ApplyInfiniteEffect = EApplyEffectPolicy::DoNotApply;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects")
	ERemoveEffectPolicy RemoveInfiniteEffect = ERemoveEffectPolicy::RemoveOnEndOverlap;

	TMap<FActiveGameplayEffectHandle, UAbilitySystemComponent*> ActiveEffectHandles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Effects")
	TArray<FEffectType> Effects;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Effects")
	float ActorLevel = 1.f;

private:

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;

};
