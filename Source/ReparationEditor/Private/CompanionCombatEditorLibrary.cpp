#include "CompanionCombatEditorLibrary.h"
#include "WidgetBlueprint.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"

FString UCompanionCombatEditorLibrary::DescribeWidgetRoot(UBlueprint* Blueprint)
{
	const UWidgetBlueprint* Widget = Cast<UWidgetBlueprint>(Blueprint);
	return Widget && Widget->WidgetTree && Widget->WidgetTree->RootWidget ? Widget->WidgetTree->RootWidget->GetClass()->GetName() : TEXT("None");
}

FString UCompanionCombatEditorLibrary::DescribeWidgetLayout(UBlueprint* Blueprint)
{
	const UWidgetBlueprint* Widget = Cast<UWidgetBlueprint>(Blueprint);
	if (!Widget || !Widget->WidgetTree) return TEXT("No widget tree");
	FString Report;
	Widget->WidgetTree->ForEachWidget([&Report](UWidget* Child)
	{
		Report += FString::Printf(TEXT("%s [%s] parent=%s visibility=%d"), *Child->GetName(), *Child->GetClass()->GetName(), *GetNameSafe(Child->GetParent()), static_cast<int32>(Child->GetVisibility()));
		if (const UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Child->Slot))
		{
			const FAnchors Anchors = Slot->GetAnchors();
			const FMargin Offsets = Slot->GetOffsets();
			Report += FString::Printf(TEXT(" anchors=(%.2f,%.2f)-(%.2f,%.2f) offsets=(%.1f,%.1f,%.1f,%.1f)"), Anchors.Minimum.X, Anchors.Minimum.Y, Anchors.Maximum.X, Anchors.Maximum.Y, Offsets.Left, Offsets.Top, Offsets.Right, Offsets.Bottom);
		}
		if (const UTextBlock* Text = Cast<UTextBlock>(Child)) Report += TEXT(" text=") + Text->GetText().ToString();
		Report += TEXT("\n");
	});
	return Report;
}
#include "Animation/AnimBlueprint.h"
#include "AnimGraphNode_Root.h"
#include "AnimGraphNode_Slot.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphSchema.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "AI/RCompanion.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/SkeletalMeshSocket.h"
#include "FileHelpers.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Engine/CurveTable.h"

FString UCompanionCombatEditorLibrary::DescribeDamageCurves(UCurveTable* Table)
{
	if (!Table) return TEXT("Damage coefficient table missing.");
	FString Report;
	for (FName Name : {FName(TEXT("ArmorPenetration")), FName(TEXT("EffectiveArmor")), FName(TEXT("CriticalHitResistance"))})
	{
		const FRealCurve* Curve = Table->FindCurve(Name, FString(), false);
		Report += Name.ToString();
		if (!Curve) { Report += TEXT(": MISSING\n"); continue; }
		for (int32 Level : {1, 5, 10, 20, 50, 100}) Report += FString::Printf(TEXT(" level%d=%.4f"), Level, Curve->Eval(Level));
		Report += TEXT("\n");
	}
	return Report;
}

FString UCompanionCombatEditorLibrary::GetActorPackageFile(ARCompanion* Companion)
{
	if (!Companion) return FString();
	UPackage* Package = Companion->GetExternalPackage() ? Companion->GetExternalPackage() : Companion->GetOutermost();
	return FPaths::ConvertRelativePathToFull(FPackageName::LongPackageNameToFilename(Package->GetName(), FPackageName::GetAssetPackageExtension()));
}

bool UCompanionCombatEditorLibrary::RestoreEmptyWeaponSlots(ARCompanion* Companion)
{
	if (!Companion || Companion->HasAnyFlags(RF_ClassDefaultObject)) return false;
	const ARCompanion* Defaults = Companion->GetClass()->GetDefaultObject<ARCompanion>();
	Companion->Modify();
	auto RestoreSlot = [](USkeletalMeshComponent* Slot, const USkeletalMeshComponent* DefaultSlot)
	{
		if (Slot && DefaultSlot && !Slot->GetSkeletalMeshAsset() && DefaultSlot->GetSkeletalMeshAsset())
		{
			Slot->Modify();
			Slot->SetSkeletalMeshAsset(DefaultSlot->GetSkeletalMeshAsset());
		}
	};
	RestoreSlot(Companion->RightWeaponSlot, Defaults->RightWeaponSlot);
	RestoreSlot(Companion->LeftWeaponSlot, Defaults->LeftWeaponSlot);
	Companion->RefreshWeaponAttachments();
	if (!Companion->HasAssignedWeapon()) return false;
	UPackage* Package = Companion->GetExternalPackage() ? Companion->GetExternalPackage() : Companion->GetOutermost();
	Package->MarkPackageDirty();
	return UEditorLoadingAndSavingUtils::SavePackages({Package}, true);
}

bool UCompanionCombatEditorLibrary::ConfigureWeaponSlots(UBlueprint* Blueprint, USkeletalMesh* RightMesh, USkeletalMesh* LeftMesh, bool bRequireWeapon)
{
	ARCompanion* Defaults = Blueprint && Blueprint->GeneratedClass ? Cast<ARCompanion>(Blueprint->GeneratedClass->GetDefaultObject()) : nullptr;
	USkeletalMesh* Body = Defaults ? Defaults->GetMesh()->GetSkeletalMeshAsset() : nullptr;
	if (!Body || !Defaults->RightWeaponSlot || !Defaults->LeftWeaponSlot) return false;
	// Mesh-only sockets avoid changing the shared MetaHuman skeleton.
	auto EnsureSocket = [Body](FName Name, FName Bone)
	{
		if (Body->FindSocket(Name)) return true;
		if (Body->GetRefSkeleton().FindBoneIndex(Bone) == INDEX_NONE) return false;
		Body->Modify();
		USkeletalMeshSocket* Socket = NewObject<USkeletalMeshSocket>(Body, NAME_None, RF_Transactional);
		Socket->SocketName = Name;
		Socket->BoneName = Bone;
		Body->AddSocket(Socket, false);
		Body->MarkPackageDirty();
		return true;
	};
	if (!EnsureSocket(Defaults->RightWeaponSocket, TEXT("hand_r")) || !EnsureSocket(Defaults->LeftWeaponSocket, TEXT("hand_l"))) return false;
	Blueprint->Modify(); Defaults->Modify();
	Defaults->RightWeaponSlot->Modify(); Defaults->LeftWeaponSlot->Modify();
	Defaults->RightWeaponSlot->SetSkeletalMeshAsset(RightMesh);
	Defaults->LeftWeaponSlot->SetSkeletalMeshAsset(LeftMesh);
	Defaults->bRequiresEquippedWeapon = bRequireWeapon;
	Defaults->RefreshWeaponAttachments();
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return Blueprint->Status != BS_Error;
}

FGameplayTag UCompanionCombatEditorLibrary::GetRegisteredTag(FName TagName)
{
	return FGameplayTag::RequestGameplayTag(TagName);
}

bool UCompanionCombatEditorLibrary::EnsureMontageSlot(UAnimBlueprint* Blueprint, FName SlotName)
{
	if (!Blueprint || SlotName.IsNone()) return false;
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	for (UEdGraph* Graph : Graphs)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			UAnimGraphNode_Root* Root = Cast<UAnimGraphNode_Root>(Node);
			if (!Root) continue;
			UEdGraphPin* Result = Root->FindPin(TEXT("Result"), EGPD_Input);
			if (!Result || Result->LinkedTo.Num() != 1) return false;
			UEdGraphPin* OriginalPose = Result->LinkedTo[0];
			if (UAnimGraphNode_Slot* Existing = Cast<UAnimGraphNode_Slot>(OriginalPose->GetOwningNode()))
				return Existing->Node.SlotName == SlotName;
			Blueprint->Modify();
			Graph->Modify();
			Root->Modify();
			FGraphNodeCreator<UAnimGraphNode_Slot> Creator(*Graph);
			UAnimGraphNode_Slot* Slot = Creator.CreateNode();
			Slot->Node.SlotName = SlotName;
			Slot->NodePosX = Root->NodePosX - 220;
			Slot->NodePosY = Root->NodePosY;
			Creator.Finalize();
			UEdGraphPin* Source = Slot->FindPin(TEXT("Source"), EGPD_Input);
			UEdGraphPin* Pose = Slot->FindPin(TEXT("Pose"), EGPD_Output);
			const UEdGraphSchema* Schema = Graph->GetSchema();
			if (!Source || !Pose || !Schema)
			{
				Graph->RemoveNode(Slot);
				return false;
			}
			Result->BreakAllPinLinks();
			if (!Schema->TryCreateConnection(OriginalPose, Source) || !Schema->TryCreateConnection(Pose, Result))
			{
				Graph->RemoveNode(Slot);
				Schema->TryCreateConnection(OriginalPose, Result);
				return false;
			}
			FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
			FKismetEditorUtilities::CompileBlueprint(Blueprint);
			return Blueprint->Status != BS_Error;
		}
	}
	return false;
}
