// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PawnCombatComponent.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPARATION_API UPawnCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	virtual void OnHitTargetActor(AActor* HitActor);

	UPROPERTY()
	TArray<AActor*> OverlappedActors;

protected:
	template<class T>
	T* GetOwningPawn() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value, "'T' must be derived from 'APawn'");
		return CastChecked<T>(GetOwner());
	}
	APawn* GetOwningPawn() const
	{
		return GetOwningPawn<APawn>();
	}
	template<class T>
	T* GetOwningController() const
	{
		static_assert(TPointerIsConvertibleFromTo<T, AController>::Value, "'T' must be derived from 'AController'");
		return GetOwningPawn<APawn>()->GetController<T>();
	}
};
