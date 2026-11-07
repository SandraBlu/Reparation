#pragma once
#include "Animation/AnimNotifies/AnimNotify.h"
#include "RAnimNotify_AssassinationHit.generated.h"

/** Place at the lethal contact frame of an assassination montage. */
UCLASS(meta=(DisplayName="Assassination Hit"))
class REPARATION_API URAnimNotify_AssassinationHit : public UAnimNotify
{
    GENERATED_BODY()
public:
    URAnimNotify_AssassinationHit();
    virtual void Notify(USkeletalMeshComponent* Mesh,UAnimSequenceBase* Animation,const FAnimNotifyEventReference& Reference) override;
    virtual FString GetNotifyName_Implementation() const override { return TEXT("Assassination Hit"); }
};
