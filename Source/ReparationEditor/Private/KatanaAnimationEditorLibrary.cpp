#include "KatanaAnimationEditorLibrary.h"
#include "Animation/Skeleton.h"

bool UKatanaAnimationEditorLibrary::SetPelvisAnimationScaled(USkeleton* Skeleton)
{
    if (!Skeleton || !Skeleton->GetPathName().StartsWith(TEXT("/Game/"))) return false;
    const int32 Index = Skeleton->GetReferenceSkeleton().FindBoneIndex(TEXT("pelvis"));
    if (Index == INDEX_NONE) return false;
    if (Skeleton->GetBoneTranslationRetargetingMode(Index) != EBoneTranslationRetargetingMode::AnimationScaled)
    {
        Skeleton->Modify();
        Skeleton->SetBoneTranslationRetargetingMode(Index,EBoneTranslationRetargetingMode::AnimationScaled,false);
        Skeleton->PostEditChange();
        Skeleton->MarkPackageDirty();
    }
    return true;
}
