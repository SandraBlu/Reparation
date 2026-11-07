#pragma once
#include "Actors/RWeapon.h"
#include "RBow.generated.h"

class UPoseableMeshComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FRBowReleased, float, Charge);

/** Inventory-compatible bow with an independently posed weapon rig. */
UCLASS()
class REPARATION_API ARBow : public ARWeapon
{
    GENERATED_BODY()
public:
    ARBow();
    virtual void OnConstruction(const FTransform& Transform) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void OnUnEquip() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Bow") TObjectPtr<UPoseableMeshComponent> BowPose;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Bow") TObjectPtr<UStaticMeshComponent> NockedArrow;
    /** Mesh placement relative to the equipped weapon socket; leave source assets unchanged. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") FTransform MeshOffset;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") FName StringBone = TEXT("bow_string_mid");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") FName DrawingHandSocket = TEXT("hand_r");
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") bool bFollowDrawingHand = true;
    /** Component-space fallback direction, also used without an owning character. */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") FVector DrawOffset = FVector(-40.f,0.f,0.f);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig", meta=(ClampMin="0")) float MaximumStringTravel = 80.f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") TArray<FName> UpperLimbBones;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") TArray<FName> LowerLimbBones;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") FRotator UpperLimbBend = FRotator(0.f,0.f,4.f);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") FRotator LowerLimbBend = FRotator(0.f,0.f,-4.f);
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig", meta=(ClampMin="0.01")) float StringReturnSeconds = .08f;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Rig") FTransform ArrowOffset;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Audio") TObjectPtr<USoundBase> DrawSound;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Bow|Audio") TObjectPtr<USoundBase> ReleaseSound;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Bow") float DrawAmount = 0.f;
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Bow") bool bDrawing = false;
    /** Ability/input may hold aim independently of the string draw. */
    UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Bow") bool bAiming = false;
    UFUNCTION(BlueprintCallable, Category="Bow") void SetAiming(bool bEnabled) { bAiming = bEnabled && IsEquipped; }
    /** Called exactly once by a release notify; a bow ability can use Charge for its shot. */
    UPROPERTY(BlueprintAssignable, Category="Bow") FRBowReleased OnArrowReleased;

    UFUNCTION(BlueprintCallable, Category="Bow") bool BeginDraw(float Duration = 1.f);
    UFUNCTION(BlueprintCallable, Category="Bow") void ReleaseString();
    UFUNCTION(BlueprintCallable, Category="Bow") void CancelDraw();
    /** Deterministic pose function for Blueprint previews and animation-driven charge. */
    UFUNCTION(BlueprintCallable, Category="Bow") void ApplyDrawPose(float Amount);
    UFUNCTION(BlueprintPure, Category="Bow") FVector GetArrowReleaseLocation() const;
protected:
    virtual void BeginPlay() override;
private:
    void ConfigureVisuals();
    float DrawDuration = 1.f;
    float DrawElapsed = 0.f;
    FVector LastStringTarget = FVector::ZeroVector;
    bool bHasStringTarget = false;
};
