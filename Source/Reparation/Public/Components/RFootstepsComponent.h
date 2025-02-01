// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RFootstepsComponent.generated.h"


enum class EFoot : uint8;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class REPARATION_API URFootstepsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	
	URFootstepsComponent();

	void HandleFootstep(EFoot Foot);

protected:
	
	UPROPERTY(EditDefaultsOnly)
	FName LeftFootSocket = TEXT("foot_l");

	UPROPERTY(EditDefaultsOnly)
	FName RightFootSocket = TEXT("foot_r");	

		
};
