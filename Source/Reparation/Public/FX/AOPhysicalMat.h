// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FX/AOPhysicalMaterial.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AOPhysicalMat.generated.h"

/**
 * 
 */
UCLASS()
class REPARATION_API UAOPhysicalMat : public UAOPhysicalMaterial
{
	GENERATED_BODY()

	public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	class USoundBase* FootstepSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
	class UNiagaraSystem* FootstepVFX = nullptr;
	
};
