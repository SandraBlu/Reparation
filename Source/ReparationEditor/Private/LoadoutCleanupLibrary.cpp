#include "CompanionCombatEditorLibrary.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintEditorUtils.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"

// One-time, narrowly scoped migration of the original menu. The equipment paths stay authored in UMG.
bool UCompanionCombatEditorLibrary::CleanLegacyLoadout()
{
    UWidgetBlueprint* BP = LoadObject<UWidgetBlueprint>(nullptr, TEXT("/Game/UI/Abilities/UI_AbilityMenu"));
    if (!BP || !BP->WidgetTree) return false;
    BP->Modify();
    TArray<UEdGraph*> Graphs;
    BP->GetAllGraphs(Graphs);
    auto Remove = [BP, &Graphs](const TCHAR* GraphName, std::initializer_list<const TCHAR*> Names)
    {
        for (UEdGraph* Graph : Graphs) if (Graph->GetFName() == FName(GraphName))
            for (const TCHAR* Name : Names)
                for (UEdGraphNode* Node : TArray<TObjectPtr<UEdGraphNode>>(Graph->Nodes))
                    if (Node->GetFName() == FName(Name))
                    {
                        // Bypass execution so deleting the old spend binding preserves the equip binding after it.
                        UEdGraphPin* In = Node->FindPin(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
                        UEdGraphPin* Out = Node->FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output);
                        if (In && Out)
                        {
                            const TArray<UEdGraphPin*> Before = In->LinkedTo, After = Out->LinkedTo;
                            In->BreakAllPinLinks(); Out->BreakAllPinLinks();
                            for (UEdGraphPin* A : Before) for (UEdGraphPin* B : After) A->MakeLinkTo(B);
                        }
                        FBlueprintEditorUtils::RemoveNode(BP, Node, true);
                    }
    };
    // Exact nodes audited in Saved/LoadoutCleanupAudit.txt and LoadoutFunctionsAudit.txt.
    Remove(TEXT("EventGraph"), {TEXT("K2Node_AssignDelegate_4"), TEXT("K2Node_CustomEvent_4"), TEXT("K2Node_CallFunction_8"),
        TEXT("K2Node_VariableGet_12"), TEXT("K2Node_VariableGet_13"), TEXT("K2Node_VariableGet_16"), TEXT("EdGraphNode_Comment_7"),
        TEXT("K2Node_AssignDelegate_2"), TEXT("K2Node_CustomEvent_2"), TEXT("K2Node_CallFunction_11"),
        TEXT("K2Node_VariableGet_14"), TEXT("K2Node_ClearDelegate_1"), TEXT("K2Node_Knot_0"), TEXT("EdGraphNode_Comment_10")});
    Remove(TEXT("SetMenuButtonsEnabled"), {TEXT("K2Node_CallFunction_10"), TEXT("K2Node_VariableGet_17")});
    Remove(TEXT("SetDescriptions"), {TEXT("K2Node_CallFunction_9"), TEXT("K2Node_CallFunction_17"), TEXT("K2Node_VariableGet_18")});
    for (UEdGraph* Graph : Graphs) if (Graph->GetFName() == TEXT("SkillPointsChange"))
        FBlueprintEditorUtils::RemoveGraph(BP, Graph, EGraphRemoveFlags::Recompile);
    TSet<UWidget*> Obsolete;
    for (const TCHAR* Name : {TEXT("TitleBox"), TEXT("SizeBox")})
        if (UWidget* Widget = BP->WidgetTree->FindWidget(FName(Name))) Obsolete.Add(Widget);
    FWidgetBlueprintEditorUtils::DeleteWidgets(BP, Obsolete, FWidgetBlueprintEditorUtils::EDeleteWidgetWarningType::DeleteSilently);
    if (UTextBlock* Heading = Cast<UTextBlock>(BP->WidgetTree->FindWidget(TEXT("TextBlock_415"))))
        Heading->SetText(FText::FromString(TEXT("Active Skills")));
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
    FKismetEditorUtilities::CompileBlueprint(BP);
    return BP->Status != BS_Error;
}
