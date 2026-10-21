#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RSneakArea.generated.h"

class UBoxComponent;
class UInstancedStaticMeshComponent;
class UStaticMesh;

/** Gameplay boundary for concealing vegetation. Visual foliage does not need collision. */
UCLASS()
class REPARATION_API ARSneakArea : public AActor
{
	GENERATED_BODY()
public:
	ARSneakArea();
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sneak Area")
	TObjectPtr<UBoxComponent> Bounds;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Sneak Area")
	TObjectPtr<UInstancedStaticMeshComponent> Vegetation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sneak Area")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sneak Area")
	bool bRequiresCrouch = true;

	/** Legacy setting retained for saved assets; concealed cover now blocks all sight. */
	UPROPERTY(meta=(DeprecatedProperty, DeprecationMessage="Concealment now blocks sight at all distances."))
	float RevealDistance = 200.f;

	/** Optional. Leave empty when using Landscape Grass or painted foliage. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sneak Area|Vegetation")
	TObjectPtr<UStaticMesh> VegetationMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sneak Area|Vegetation", meta=(ClampMin="0", ClampMax="2000"))
	int32 InstanceCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sneak Area|Vegetation")
	int32 RandomSeed = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Sneak Area|Vegetation", meta=(ClampMin="0.01"))
	float VegetationScale = 1.f;

	UFUNCTION(BlueprintPure, Category="Sneak Area")
	bool ContainsLocation(FVector Location) const;
};
