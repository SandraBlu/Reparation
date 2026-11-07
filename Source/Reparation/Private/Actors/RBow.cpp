#include "Actors/RBow.h"
#include "Characters/RPlayer.h"
#include "Components/PoseableMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "NativeGameplayTags.h"
#include "UObject/ConstructorHelpers.h"

UE_DEFINE_GAMEPLAY_TAG_STATIC(BowWeaponTag,"weapon.bow");

ARBow::ARBow()
{
    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.TickGroup = TG_PostUpdateWork;
    CombatType = ECombatType::ECT_Ranged;
    WeaponTag = BowWeaponTag;
    ActiveWeaponSocket = TEXT("weapon_l");
    InactiveWeaponSocket = TEXT("spine_03");
    FiringSocket = TEXT("bow_string_mid");
    BowPose = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("BowPose"));
    BowPose->SetupAttachment(WeaponMesh);
    BowPose->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NockedArrow = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("NockedArrow"));
    NockedArrow->SetupAttachment(BowPose, StringBone);
    NockedArrow->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    NockedArrow->SetVisibility(false);
    UpperLimbBones = {TEXT("bow_shaft_top_01"),TEXT("bow_shaft_top_02"),TEXT("bow_shaft_top_03")};
    LowerLimbBones = {TEXT("bow_shaft_bott_01"),TEXT("bow_shaft_bott_02"),TEXT("bow_shaft_bott_03")};
    static ConstructorHelpers::FObjectFinder<USkeletalMesh> Mesh(TEXT("/Game/ResourcePack/Weapons/Meshes/SK_ElvenBow"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Arrow(TEXT("/Game/Inventory/Weapon/Mesh/Bow/SM_Arrow_01"));
    if (Mesh.Succeeded()) WeaponMesh->SetSkeletalMeshAsset(Mesh.Object);
    if (Arrow.Succeeded()) NockedArrow->SetStaticMesh(Arrow.Object);
}

void ARBow::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    ConfigureVisuals();
}

void ARBow::ConfigureVisuals()
{
    BowPose->SetSkinnedAssetAndUpdate(WeaponMesh->GetSkeletalMeshAsset());
    BowPose->SetRelativeTransform(MeshOffset);
    // Retain the inventory's standard skeletal weapon root while rendering its posed child.
    WeaponMesh->SetVisibility(false, false);
    BowPose->SetVisibility(true);
    NockedArrow->AttachToComponent(BowPose, FAttachmentTransformRules::KeepRelativeTransform, StringBone);
    NockedArrow->SetRelativeTransform(ArrowOffset);
    ApplyDrawPose(0.f);
}

void ARBow::BeginPlay()
{
    Super::BeginPlay();
    ConfigureVisuals();
    if (PawnOwner) AddTickPrerequisiteComponent(PawnOwner->GetMesh());
}

bool ARBow::BeginDraw(float Duration)
{
    if (!IsEquipped || bDrawing || !FMath::IsFinite(Duration) || Duration <= 0.f) return false;
    DrawDuration = FMath::Max(.01f, Duration);
    DrawElapsed = 0.f;
    bDrawing = true;
    ApplyDrawPose(0.f);
    NockedArrow->SetVisibility(true);
    if (DrawSound) UGameplayStatics::PlaySoundAtLocation(this, DrawSound, GetActorLocation());
    return true;
}

void ARBow::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!IsEquipped) { if (bDrawing || bAiming || DrawAmount > 0.f) CancelDraw(); return; }
    if (bDrawing)
    {
        DrawElapsed += DeltaSeconds;
        ApplyDrawPose(FMath::Clamp(DrawElapsed / DrawDuration, 0.f, 1.f));
    }
    else if (DrawAmount > 0.f)
        ApplyDrawPose(FMath::Max(0.f, DrawAmount - DeltaSeconds / FMath::Max(.01f,StringReturnSeconds)));
}

void ARBow::ApplyDrawPose(float Amount)
{
    DrawAmount = FMath::IsFinite(Amount) ? FMath::Clamp(Amount,0.f,1.f) : 0.f;
    const USkeletalMesh* Mesh = Cast<USkeletalMesh>(BowPose->GetSkinnedAsset());
    if (!Mesh) return;
    const FReferenceSkeleton& Ref = Mesh->GetRefSkeleton();
    BowPose->BoneSpaceTransforms = Ref.GetRefBonePose();
    auto Bend = [&](const TArray<FName>& Bones, const FRotator& Angle)
    {
        const FQuat Rotation = (Angle * DrawAmount).Quaternion();
        for (FName Bone : Bones)
        {
            const int32 Index = Ref.FindBoneIndex(Bone);
            if (BowPose->BoneSpaceTransforms.IsValidIndex(Index))
            {
                auto& Transform = BowPose->BoneSpaceTransforms[Index];
                Transform.SetRotation((Rotation * Transform.GetRotation()).GetNormalized());
            }
        }
    };
    Bend(UpperLimbBones, UpperLimbBend);
    Bend(LowerLimbBones, LowerLimbBend);
    // Build component transforms explicitly; never accumulate the last frame's deformation.
    TArray<FTransform> ComponentPose;
    ComponentPose.SetNum(Ref.GetNum());
    for (int32 Index=0; Index<Ref.GetNum(); ++Index)
    {
        const int32 Parent = Ref.GetParentIndex(Index);
        ComponentPose[Index] = Parent == INDEX_NONE ? BowPose->BoneSpaceTransforms[Index]
            : BowPose->BoneSpaceTransforms[Index] * ComponentPose[Parent];
    }
    const int32 StringIndex = Ref.FindBoneIndex(StringBone);
    if (ComponentPose.IsValidIndex(StringIndex))
    {
        const FVector Rest = ComponentPose[StringIndex].GetLocation();
        FVector Target = !bDrawing && bHasStringTarget ? LastStringTarget : Rest + DrawOffset;
        if (bDrawing && bFollowDrawingHand && PawnOwner && PawnOwner->GetMesh()->DoesSocketExist(DrawingHandSocket))
            Target = BowPose->GetComponentTransform().InverseTransformPosition(PawnOwner->GetMesh()->GetSocketLocation(DrawingHandSocket));
        if (bDrawing) { LastStringTarget = Target; bHasStringTarget = true; }
        const FVector Position = Rest + (Target-Rest).GetClampedToMaxSize(FMath::Max(0.f,MaximumStringTravel)) * DrawAmount;
        const int32 Parent = Ref.GetParentIndex(StringIndex);
        BowPose->BoneSpaceTransforms[StringIndex].SetLocation(Parent==INDEX_NONE ? Position : ComponentPose[Parent].InverseTransformPosition(Position));
    }
    BowPose->MarkRefreshTransformDirty();
    BowPose->RefreshBoneTransforms();
}

FVector ARBow::GetArrowReleaseLocation() const
{
    return BowPose->DoesSocketExist(StringBone) ? BowPose->GetSocketLocation(StringBone) : GetActorLocation();
}

void ARBow::ReleaseString()
{
    if (!bDrawing || !IsEquipped) return;
    const float Charge = DrawAmount;
    bDrawing = false;
    NockedArrow->SetVisibility(false);
    if (ReleaseSound) UGameplayStatics::PlaySoundAtLocation(this, ReleaseSound, GetActorLocation());
    OnArrowReleased.Broadcast(Charge);
}

void ARBow::CancelDraw()
{
    bAiming = false;
    bDrawing = false;
    DrawElapsed = 0.f;
    bHasStringTarget = false;
    NockedArrow->SetVisibility(false);
    ApplyDrawPose(0.f);
}

void ARBow::OnUnEquip()
{
    CancelDraw();
    Super::OnUnEquip();
}
