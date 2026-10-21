#include "EnemyBlueprintEditorLibrary.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_CallFunction.h"
#include "K2Node_IfThenElse.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "GAS/Ability/RMeleeAbility.h"
#include "Framework/RAbilitySystemLibrary.h"

bool UEnemyBlueprintEditorLibrary::WireSuccessfulBlockNotification(UBlueprint* Blueprint)
{
    if (!Blueprint) return false;
    TArray<UEdGraph*> Graphs; Blueprint->GetAllGraphs(Graphs);
    for (UEdGraph* Graph : Graphs)
    {
        UK2Node_CallFunction* Direction=nullptr;
        for (UEdGraphNode* Node : Graph->Nodes) if (auto* Call=Cast<UK2Node_CallFunction>(Node))
        {
            if (Call->FunctionReference.GetMemberName()==TEXT("NotifySuccessfulBlock")) return true;
            if (Call->FunctionReference.GetMemberName()==TEXT("IsValidBlock")) Direction=Call;
        }
        if (!Direction) continue;
        UEdGraphPin* Result=Direction->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
        UK2Node_IfThenElse* Branch=nullptr;
        for (UEdGraphNode* Node : Graph->Nodes) if (auto* Candidate=Cast<UK2Node_IfThenElse>(Node))
            for (UEdGraphPin* Link : Candidate->GetConditionPin()->LinkedTo)
                for (UEdGraphPin* Input : Link->GetOwningNode()->Pins)
                    if (Input->LinkedTo.Contains(Result)) Branch=Candidate;
        auto* Attacker=Direction->FindPin(TEXT("InAttacker")); auto* Defender=Direction->FindPin(TEXT("InDefender"));
        if (!Branch || Branch->GetThenPin()->LinkedTo.Num()!=1 || Attacker->LinkedTo.Num()!=1 || Defender->LinkedTo.Num()!=1) return false;
        UEdGraphPin* Next=Branch->GetThenPin()->LinkedTo[0];
        FGraphNodeCreator<UK2Node_CallFunction> Creator(*Graph);
        auto* Call=Creator.CreateNode(); Call->SetFromFunction(URAbilitySystemLibrary::StaticClass()->FindFunctionByName(TEXT("NotifySuccessfulBlock")));
        Call->NodePosX=Branch->NodePosX+220; Call->NodePosY=Branch->NodePosY-100; Creator.Finalize();
        const auto* Schema=Graph->GetSchema(); Branch->GetThenPin()->BreakAllPinLinks();
        if (!Schema->TryCreateConnection(Branch->GetThenPin(),Call->FindPin(UEdGraphSchema_K2::PN_Execute))
            || !Schema->TryCreateConnection(Call->FindPin(UEdGraphSchema_K2::PN_Then),Next)
            || !Schema->TryCreateConnection(Attacker->LinkedTo[0],Call->FindPin(TEXT("Attacker")))
            || !Schema->TryCreateConnection(Defender->LinkedTo[0],Call->FindPin(TEXT("Defender")))) return false;
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint); FKismetEditorUtilities::CompileBlueprint(Blueprint);
        return Blueprint->Status!=BS_Error;
    }
    return false;
}

bool UEnemyBlueprintEditorLibrary::WireKatanaComboCompletion(UBlueprint* Blueprint)
{
    if (!Blueprint || !Blueprint->GeneratedClass->IsChildOf(URMeleeAbility::StaticClass())) return false;
    TArray<UEdGraph*> Graphs; Blueprint->GetAllGraphs(Graphs);
    for (UEdGraph* Graph : Graphs)
    {
        UEdGraphPin* Completed = nullptr;
        UEdGraphPin* Play = nullptr;
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node))
                if (Call->FunctionReference.GetMemberName() == TEXT("AdvanceCombo")) return true;
            // Match the existing montage task rather than replacing its animation or hit listeners.
            if (Node->FindPin(TEXT("MontageToPlay")))
            {
                Completed = Node->FindPin(TEXT("OnCompleted"));
                Play = Node->FindPin(UEdGraphSchema_K2::PN_Execute);
            }
        }
        if (!Completed || !Play || Completed->LinkedTo.Num() != 1) continue;
        UEdGraphPin* End = Completed->LinkedTo[0];
        Graph->Modify();
        FGraphNodeCreator<UK2Node_CallFunction> AdvanceCreator(*Graph);
        UK2Node_CallFunction* Advance = AdvanceCreator.CreateNode();
        Advance->SetFromFunction(URMeleeAbility::StaticClass()->FindFunctionByName(TEXT("AdvanceCombo")));
        Advance->NodePosX = Completed->GetOwningNode()->NodePosX + 450;
        Advance->NodePosY = Completed->GetOwningNode()->NodePosY - 150;
        AdvanceCreator.Finalize();
        FGraphNodeCreator<UK2Node_IfThenElse> BranchCreator(*Graph);
        UK2Node_IfThenElse* Branch = BranchCreator.CreateNode();
        Branch->NodePosX = Advance->NodePosX + 260;
        Branch->NodePosY = Advance->NodePosY;
        BranchCreator.Finalize();
        Completed->BreakAllPinLinks();
        const UEdGraphSchema* Schema = Graph->GetSchema();
        const bool Success = Schema->TryCreateConnection(Completed, Advance->FindPin(UEdGraphSchema_K2::PN_Execute))
            && Schema->TryCreateConnection(Advance->FindPin(UEdGraphSchema_K2::PN_Then), Branch->GetExecPin())
            && Schema->TryCreateConnection(Advance->FindPin(UEdGraphSchema_K2::PN_ReturnValue), Branch->GetConditionPin())
            && Schema->TryCreateConnection(Branch->GetThenPin(), Play)
            && Schema->TryCreateConnection(Branch->GetElsePin(), End);
        if (!Success) return false;
        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
        FKismetEditorUtilities::CompileBlueprint(Blueprint);
        return Blueprint->Status != BS_Error;
    }
    return false;
}
