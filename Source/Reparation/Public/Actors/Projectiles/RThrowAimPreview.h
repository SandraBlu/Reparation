#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RThrowAimPreview.generated.h"

/** Local rendered trajectory and blast-radius marker; has no collision or gameplay effects. */
UCLASS()
class REPARATION_API ARThrowAimPreview : public AActor
{
    GENERATED_BODY()
public:
    ARThrowAimPreview();
    void UpdateTrajectory(FVector Origin, FVector Velocity, float GravityScale, float BlastRadius, float ProjectileRadius);
    UPROPERTY(EditDefaultsOnly, Category="Aim") TObjectPtr<class UMaterialInterface> PreviewMaterial;
    UPROPERTY(EditDefaultsOnly, Category="Aim") float LineRadius = 1.5f;
    UPROPERTY(BlueprintReadOnly, Category="Aim") bool bHasTarget = false;
    UPROPERTY(BlueprintReadOnly, Category="Aim") FVector TargetLocation = FVector::ZeroVector;
private:
    UPROPERTY() TObjectPtr<class UStaticMesh> SegmentMesh;
    UPROPERTY() TArray<TObjectPtr<class UStaticMeshComponent>> Segments;
    void DrawSegment(int32 Index, FVector Start, FVector End);
};
