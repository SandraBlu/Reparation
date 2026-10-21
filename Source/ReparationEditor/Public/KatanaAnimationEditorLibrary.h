#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "KatanaAnimationEditorLibrary.generated.h"

class USkeleton;

/** Targeted authoring tools; never modify engine/plugin skeleton assets. */
UCLASS()
class REPARATIONEDITOR_API UKatanaAnimationEditorLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    /** Preserve animated pelvis translation without recursively changing child bones. Does not save. */
    UFUNCTION(BlueprintCallable, Category="Combat|Editor")
    static bool SetPelvisAnimationScaled(USkeleton* Skeleton);
};
