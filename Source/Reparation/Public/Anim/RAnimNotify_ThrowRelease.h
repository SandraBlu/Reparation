#pragma once
#include "Animation/AnimNotifies/AnimNotify.h"
#include "RAnimNotify_ThrowRelease.generated.h"

/** Drag this notify to the frame where the throwable leaves the hand. */
UCLASS(meta=(DisplayName="Throw Release"))
class REPARATION_API URAnimNotify_ThrowRelease : public UAnimNotify
{
    GENERATED_BODY()
public:
    URAnimNotify_ThrowRelease();
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
        const FAnimNotifyEventReference& EventReference) override;
    virtual FString GetNotifyName_Implementation() const override { return TEXT("Throw Release"); }
};
