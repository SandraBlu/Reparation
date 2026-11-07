#pragma once
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "RAnimNotify_Bow.generated.h"

/** Place from nocking through release. Ending/interruption resets the weapon pose. */
UCLASS(meta=(DisplayName="Bow Draw"))
class REPARATION_API URAnimNotifyState_BowDraw : public UAnimNotifyState
{
    GENERATED_BODY()
public:
    virtual void NotifyBegin(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, float Duration, const FAnimNotifyEventReference& Reference) override;
    virtual void NotifyEnd(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& Reference) override;
};

/** Place at the exact release frame, inside the Bow Draw state. */
UCLASS(meta=(DisplayName="Bow Release"))
class REPARATION_API URAnimNotify_BowRelease : public UAnimNotify
{
    GENERATED_BODY()
public:
    URAnimNotify_BowRelease();
    virtual void Notify(USkeletalMeshComponent* Mesh, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& Reference) override;
};
