#include "EnemyBlueprintEditorLibrary.h"
#include "Engine/Blueprint.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "K2Node_CallFunction.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_IfThenElse.h"
#include "EdGraphSchema_K2.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "AI/REnemy.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "UObject/UnrealType.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "Anim/RAnimNotifyState_EnemyMeleeWindow.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BTService.h"
#include "BehaviorTree/BTDecorator.h"
#include "BehaviorTree/Composites/BTComposite_Sequence.h"
#include "BehaviorTreeGraph.h"
#include "BehaviorTreeGraphNode_Composite.h"
#include "BehaviorTreeGraphNode_Task.h"
#include "AI/BT/BTTask_RMeleePosition.h"
#include "K2Node_DynamicCast.h"
#include "AI/BT/BTTask_RSearch.h"
#include "AI/BT/BTService_RPerceivedTarget.h"
#include "BehaviorTree/BlackboardData.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Vector.h"
#include "BehaviorTree/Decorators/BTDecorator_Blackboard.h"
#include "BehaviorTreeGraphNode_Decorator.h"
#include "BehaviorTreeGraphNode_Service.h"

bool UEnemyBlueprintEditorLibrary::RemovePlayerMeleeHitSlowdown(UBlueprint* Blueprint)
{
	if (!Blueprint || Blueprint->GetPathName() != TEXT("/Game/GAS/GA/Player/GA_Melee.GA_Melee")) return false;
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	for (UEdGraph* Graph : Graphs)
	{
		if (Graph->GetName() != TEXT("Gameplay Ability Graph")) continue;
		auto FindCall = [&](const TCHAR* Name, const TCHAR* Function) -> UK2Node_CallFunction*
		{
			for (UEdGraphNode* Node : Graph->Nodes)
				if (Node->GetName() == Name)
					if (auto* Call = Cast<UK2Node_CallFunction>(Node))
						if (Call->FunctionReference.GetMemberName() == FName(Function)) return Call;
			return nullptr;
		};
		auto* Slow = FindCall(TEXT("K2Node_CallFunction_2"), TEXT("SetGlobalTimeDilation"));
		auto* Wait = FindCall(TEXT("K2Node_CallFunction_1"), TEXT("Delay"));
		auto* Restore = FindCall(TEXT("K2Node_CallFunction_0"), TEXT("SetGlobalTimeDilation"));
		if (!Slow || !Wait || !Restore) return false;
		auto* In = Slow->FindPin(TEXT("execute"));
		auto* Out = Restore->FindPin(TEXT("then"));
		auto* SlowOut = Slow->FindPin(TEXT("then"));
		auto* WaitIn = Wait->FindPin(TEXT("execute"));
		auto* WaitOut = Wait->FindPin(TEXT("then"));
		auto* RestoreIn = Restore->FindPin(TEXT("execute"));
		if (!In || !Out || !SlowOut || !WaitIn || !WaitOut || !RestoreIn ||
			In->LinkedTo.Num() != 1 || Out->LinkedTo.Num() != 1 ||
			SlowOut->LinkedTo.Num() != 1 || SlowOut->LinkedTo[0] != WaitIn ||
			WaitOut->LinkedTo.Num() != 1 || WaitOut->LinkedTo[0] != RestoreIn) return false;
		UEdGraphPin* Previous = In->LinkedTo[0];
		UEdGraphPin* Next = Out->LinkedTo[0];
		Blueprint->Modify(); Graph->Modify();
		Previous->GetOwningNode()->Modify(); Next->GetOwningNode()->Modify();
		In->BreakAllPinLinks(); Out->BreakAllPinLinks();
		if (!Graph->GetSchema()->TryCreateConnection(Previous, Next)) return false;
		FBlueprintEditorUtils::RemoveNode(Blueprint, Slow, true);
		FBlueprintEditorUtils::RemoveNode(Blueprint, Wait, true);
		FBlueprintEditorUtils::RemoveNode(Blueprint, Restore, true);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		return Blueprint->Status != BS_Error;
	}
	return false;
}

bool UEnemyBlueprintEditorLibrary::ConfigurePerceptionSearch(UBehaviorTree* Tree)
{
	UBehaviorTreeGraph* Graph = Tree ? Cast<UBehaviorTreeGraph>(Tree->BTGraph) : nullptr;
	if (!Graph || !Tree->BlackboardAsset) return false;
	UBehaviorTreeGraphNode_Composite* Root = nullptr;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		UBehaviorTreeGraphNode* BTNode = Cast<UBehaviorTreeGraphNode>(Node);
		if (BTNode && BTNode->NodeInstance && BTNode->NodeInstance->IsA<UBTTask_RSearch>()) return true;
		if (BTNode && BTNode->NodeInstance == Tree->RootNode) Root = Cast<UBehaviorTreeGraphNode_Composite>(Node);
	}
	if (!Root) return false;
	UEdGraphPin* RootOut = Root->FindPin(TEXT("Out"));
	if (!RootOut || RootOut->LinkedTo.Num() != 1) return false;
	UBehaviorTreeGraphNode* Combat = Cast<UBehaviorTreeGraphNode>(RootOut->LinkedTo[0]->GetOwningNode());
	if (!Combat) return false;
	Tree->Modify(); Graph->Modify(); Root->Modify();
	UBlackboardData* BB = Tree->BlackboardAsset;
	BB->Modify();
	auto AddKey = [&](FName Name, UClass* Type)
	{
		if (BB->GetKeyID(Name) != FBlackboard::InvalidKey) return;
		FBlackboardEntry Entry; Entry.EntryName = Name;
		Entry.KeyType = NewObject<UBlackboardKeyType>(BB, Type);
		BB->Keys.Add(Entry);
	};
	AddKey(TEXT("SearchActive"), UBlackboardKeyType_Bool::StaticClass());
	AddKey(TEXT("LastKnownLocation"), UBlackboardKeyType_Vector::StaticClass());
	BB->MarkPackageDirty();
	// Remove only services that select the player without consulting sight, or
	// face that independently selected player. The native root service replaces both.
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		UBehaviorTreeGraphNode* BTNode = Cast<UBehaviorTreeGraphNode>(Node);
		if (!BTNode) continue;
		const auto Services = BTNode->Services;
		for (UBehaviorTreeGraphNode* Service : Services)
			if (Service && Service->NodeInstance &&
				(Service->NodeInstance->GetClass()->GetFName() == TEXT("BTS_FindNearestPlayer_C") ||
				 Service->NodeInstance->GetClass()->GetFName() == TEXT("BTS_OrientRotationToPlayer_C")))
			{
				BTNode->Modify(); BTNode->RemoveSubNode(Service);
			}
	}
	FGraphNodeCreator<UBehaviorTreeGraphNode_Service> ServiceCreator(*Graph);
	UBehaviorTreeGraphNode_Service* Service = ServiceCreator.CreateNode();
	Service->ClassData = FGraphNodeClassData(UBTService_RPerceivedTarget::StaticClass(), FString());
	ServiceCreator.Finalize();
	Root->AddSubNode(Service, Graph);
	FGraphNodeCreator<UBehaviorTreeGraphNode_Task> TaskCreator(*Graph);
	UBehaviorTreeGraphNode_Task* Search = TaskCreator.CreateNode();
	Search->ClassData = FGraphNodeClassData(UBTTask_RSearch::StaticClass(), FString());
	Search->NodePosX = Combat->NodePosX - 500;
	Search->NodePosY = Combat->NodePosY;
	TaskCreator.Finalize();
	FGraphNodeCreator<UBehaviorTreeGraphNode_Decorator> DecoratorCreator(*Graph);
	UBehaviorTreeGraphNode_Decorator* Decorator = DecoratorCreator.CreateNode();
	Decorator->ClassData = FGraphNodeClassData(UBTDecorator_Blackboard::StaticClass(), FString());
	DecoratorCreator.Finalize();
	if (!Search->NodeInstance || !Service->NodeInstance || !Decorator->NodeInstance) return false;
	FStructProperty* KeyProperty = FindFProperty<FStructProperty>(Decorator->NodeInstance->GetClass(), TEXT("BlackboardKey"));
	FByteProperty* AbortProperty = FindFProperty<FByteProperty>(Decorator->NodeInstance->GetClass(), TEXT("FlowAbortMode"));
	if (!KeyProperty || !AbortProperty) return false;
	FBlackboardKeySelector* Key = KeyProperty->ContainerPtrToValuePtr<FBlackboardKeySelector>(Decorator->NodeInstance);
	Key->SelectedKeyName = TEXT("SearchActive");
	Key->ResolveSelectedKey(*BB);
	AbortProperty->SetPropertyValue_InContainer(Decorator->NodeInstance, EBTFlowAbortMode::Both);
	Search->AddSubNode(Decorator, Graph);
	// Preserve the combat branch's death, stun and hit-reaction gates for search.
	for (UBehaviorTreeGraphNode* Gate : Combat->Decorators)
	{
		UBTDecorator_Blackboard* BBGate = Gate ? Cast<UBTDecorator_Blackboard>(Gate->NodeInstance) : nullptr;
		if (!BBGate) continue;
		const FName GateKey = BBGate->GetSelectedBlackboardKey();
		if (GateKey != TEXT("IsDead") && GateKey != TEXT("Stunned") && GateKey != TEXT("HitReacting")) continue;
		UBehaviorTreeGraphNode* Copy = DuplicateObject<UBehaviorTreeGraphNode>(Gate, Graph);
		Copy->NodeInstance = DuplicateObject<UObject>(Gate->NodeInstance, Tree);
		Copy->CreateNewGuid();
		Search->AddSubNode(Copy, Graph);
	}
	if (!Graph->GetSchema()->TryCreateConnection(RootOut, Search->FindPin(TEXT("In")))) return false;
	Graph->UpdateAsset(UBehaviorTreeGraph::ClearDebuggerFlags);
	Tree->MarkPackageDirty();
	return true;
}

bool UEnemyBlueprintEditorLibrary::FixMeleeBlocking(UBlueprint* Blueprint)
{
	if (!Blueprint) return false;
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	bool bFound = false;
	for (UEdGraph* Graph : Graphs)
	{
		UK2Node_CallFunction* DirectionCheck = nullptr;
		UK2Node_CallFunction* LegacyBlock = nullptr;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node);
			if (!Call) continue;
			if (Call->FunctionReference.GetMemberName() == TEXT("IsActorBlocking")) bFound = true;
			if (Call->FunctionReference.GetMemberName() == TEXT("IsValidBlock")) DirectionCheck = Call;
			if (Call->FunctionReference.GetMemberName() == TEXT("GetIsUsingBlock")) LegacyBlock = Call;
		}
		if (!DirectionCheck || !LegacyBlock) continue;
		UEdGraphPin* Defender = DirectionCheck->FindPin(TEXT("InDefender"));
		UEdGraphPin* OldReturn = LegacyBlock->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
		UEdGraphPin* OldSelf = LegacyBlock->FindPin(UEdGraphSchema_K2::PN_Self);
		if (!Defender || Defender->LinkedTo.Num() != 1 || !OldReturn || OldReturn->LinkedTo.IsEmpty()) return false;
		UEdGraphPin* DefenderSource = Defender->LinkedTo[0];
		UK2Node_DynamicCast* OldCast = OldSelf && OldSelf->LinkedTo.Num() == 1 ? Cast<UK2Node_DynamicCast>(OldSelf->LinkedTo[0]->GetOwningNode()) : nullptr;
		const TArray<UEdGraphPin*> Consumers = OldReturn->LinkedTo;
		Blueprint->Modify();
		Graph->Modify();
		FGraphNodeCreator<UK2Node_CallFunction> Creator(*Graph);
		UK2Node_CallFunction* Block = Creator.CreateNode();
		Block->SetFromFunction(URAbilitySystemLibrary::StaticClass()->FindFunctionByName(TEXT("IsActorBlocking")));
		Block->NodePosX = LegacyBlock->NodePosX;
		Block->NodePosY = LegacyBlock->NodePosY;
		Block->NodeComment = TEXT("Read blocking from the hit defender; safe for players, companions and absent actors.");
		Creator.Finalize();
		const UEdGraphSchema* Schema = Graph->GetSchema();
		if (!Schema->TryCreateConnection(DefenderSource, Block->FindPin(TEXT("Defender")))) return false;
		OldReturn->BreakAllPinLinks();
		for (UEdGraphPin* Consumer : Consumers)
			if (!Schema->TryCreateConnection(Block->FindPin(UEdGraphSchema_K2::PN_ReturnValue), Consumer)) return false;
		LegacyBlock->BreakAllNodeLinks();
		Graph->RemoveNode(LegacyBlock);
		if (OldCast)
		{
			bool bUsed = false;
			for (UEdGraphPin* Pin : OldCast->Pins) bUsed |= Pin->Direction == EGPD_Output && !Pin->LinkedTo.IsEmpty();
			if (!bUsed) { OldCast->BreakAllNodeLinks(); Graph->RemoveNode(OldCast); }
		}
		bFound = true;
	}
	if (!bFound) return false;
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return Blueprint->Status != BS_Error;
}

bool UEnemyBlueprintEditorLibrary::ConfigureMaceStances(UBlueprint* Blueprint)
{
	AREnemy* Defaults = Blueprint && Blueprint->GeneratedClass ? Cast<AREnemy>(Blueprint->GeneratedClass->GetDefaultObject()) : nullptr;
	FArrayProperty* Property = Defaults ? FindFProperty<FArrayProperty>(Defaults->GetClass(), TEXT("AttackMontages")) : nullptr;
	if (!Property) return false;
	FStructProperty* Inner = CastField<FStructProperty>(Property->Inner);
	if (!Inner || Inner->Struct != FTaggedMontage::StaticStruct()) return false;
	Blueprint->Modify();
	Defaults->Modify();
	FScriptArrayHelper Array(Property, Property->ContainerPtrToValuePtr<void>(Defaults));
	bool bLow = false, bAcross = false;
	for (int32 Index = 0; Index < Array.Num(); ++Index)
	{
		FTaggedMontage& Entry = *reinterpret_cast<FTaggedMontage*>(Array.GetRawPtr(Index));
		if (Entry.Montage && Entry.Montage->GetFName() == TEXT("AM_HeavyMace")) { Entry.TargetStance = EAttackTargetStance::Crouching; bLow = true; }
		if (Entry.Montage && Entry.Montage->GetFName() == TEXT("AM_MaceAttack02")) { Entry.TargetStance = EAttackTargetStance::Standing; bAcross = true; }
	}
	if (!bLow || !bAcross) return false;
	Defaults->MinMeleeDistance = 150.f;
	Defaults->PreferredMeleeDistance = 190.f;
	Defaults->MaxMeleeDistance = 230.f;
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return Blueprint->Status != BS_Error;
}

bool UEnemyBlueprintEditorLibrary::EnsureMeleePosition(UBehaviorTree* Tree, FName AttackTaskClassName)
{
	UBehaviorTreeGraph* Graph = Tree ? Cast<UBehaviorTreeGraph>(Tree->BTGraph) : nullptr;
	if (!Graph) return false;
	TArray<UBehaviorTreeGraphNode_Task*> Attacks;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		UBehaviorTreeGraphNode_Task* Task = Cast<UBehaviorTreeGraphNode_Task>(Node);
		if (!Task || !Task->NodeInstance) continue;
		if (Task->NodeInstance->IsA<UBTTask_RMeleePosition>()) return true;
		if (Task->NodeInstance->GetClass()->GetFName() == AttackTaskClassName) Attacks.Add(Task);
	}
	if (Attacks.IsEmpty()) return false;
	Tree->Modify();
	Graph->Modify();
	for (UBehaviorTreeGraphNode_Task* Attack : Attacks)
	{
		UEdGraphPin* Input = Attack->FindPin(TEXT("In"), EGPD_Input);
		if (!Input || Input->LinkedTo.Num() != 1) return false;
		UEdGraphPin* Parent = Input->LinkedTo[0];
		FGraphNodeCreator<UBehaviorTreeGraphNode_Composite> SequenceCreator(*Graph);
		UBehaviorTreeGraphNode_Composite* Sequence = SequenceCreator.CreateNode();
		Sequence->ClassData = FGraphNodeClassData(UBTComposite_Sequence::StaticClass(), FString());
		Sequence->NodePosX = Attack->NodePosX;
		Sequence->NodePosY = Attack->NodePosY;
		SequenceCreator.Finalize();
		if (!Sequence->NodeInstance) return false;
		if (FStrProperty* Name = FindFProperty<FStrProperty>(UBTNode::StaticClass(), TEXT("NodeName")))
			Name->SetPropertyValue_InContainer(Sequence->NodeInstance, TEXT("Melee: position then attack"));
		FGraphNodeCreator<UBehaviorTreeGraphNode_Task> PositionCreator(*Graph);
		UBehaviorTreeGraphNode_Task* Position = PositionCreator.CreateNode();
		Position->ClassData = FGraphNodeClassData(UBTTask_RMeleePosition::StaticClass(), FString());
		Position->NodePosX = Sequence->NodePosX - 300;
		Position->NodePosY = Sequence->NodePosY + 200;
		PositionCreator.Finalize();
		if (!Position->NodeInstance) return false;
		FGraphNodeCreator<UBehaviorTreeGraphNode_Task> WaitCreator(*Graph);
		UBehaviorTreeGraphNode_Task* Wait = WaitCreator.CreateNode();
		Wait->ClassData = FGraphNodeClassData(UBTTask_RMeleePosition::StaticClass(), FString());
		Wait->NodePosX = Sequence->NodePosX + 300;
		Wait->NodePosY = Sequence->NodePosY + 200;
		WaitCreator.Finalize();
		UBTTask_RMeleePosition* WaitTask = Cast<UBTTask_RMeleePosition>(Wait->NodeInstance);
		if (!WaitTask) return false;
		WaitTask->bWaitForAttackOnly = true;
		if (FStrProperty* Name = FindFProperty<FStrProperty>(UBTNode::StaticClass(), TEXT("NodeName")))
			Name->SetPropertyValue_InContainer(WaitTask, TEXT("Finish committed melee swing"));
		Attack->Modify();
		Attack->NodePosX = Sequence->NodePosX;
		Attack->NodePosY = Sequence->NodePosY + 200;
		Input->BreakAllPinLinks();
		const UEdGraphSchema* Schema = Graph->GetSchema();
		if (!Schema->TryCreateConnection(Parent, Sequence->FindPin(TEXT("In"))) ||
			!Schema->TryCreateConnection(Sequence->FindPin(TEXT("Out")), Position->FindPin(TEXT("In"))) ||
			!Schema->TryCreateConnection(Sequence->FindPin(TEXT("Out")), Input) ||
			!Schema->TryCreateConnection(Sequence->FindPin(TEXT("Out")), Wait->FindPin(TEXT("In")))) return false;
	}
	Graph->UpdateAsset(UBehaviorTreeGraph::ClearDebuggerFlags);
	Tree->MarkPackageDirty();
	return true;
}

FString UEnemyBlueprintEditorLibrary::DescribeBehaviorTree(UBehaviorTree* Tree)
{
	if (!Tree || !Tree->RootNode) return TEXT("Tree missing");
	FString Report = FString::Printf(TEXT("Tree %s\n"), *Tree->GetPathName());
	TFunction<void(UBTCompositeNode*, int32)> Walk = [&](UBTCompositeNode* Composite, int32 Depth)
	{
		Report += FString::ChrN(Depth * 2, TEXT(' ')) + Composite->GetClass()->GetName() + TEXT(" ") + Composite->GetNodeName() + TEXT("\n");
		for (UBTService* Service : Composite->Services)
			if (Service) Report += FString::ChrN((Depth + 1) * 2, TEXT(' ')) + TEXT("Service ") + Service->GetClass()->GetName() + TEXT(" ") + Service->GetNodeName() + TEXT("\n");
		for (const FBTCompositeChild& Child : Composite->Children)
		{
			for (UBTDecorator* Decorator : Child.Decorators)
				if (Decorator) Report += FString::ChrN((Depth + 1) * 2, TEXT(' ')) + TEXT("Decorator ") + Decorator->GetClass()->GetName() + TEXT(" ") + Decorator->GetStaticDescription() + TEXT("\n");
			if (Child.ChildComposite) Walk(Child.ChildComposite, Depth + 1);
			if (Child.ChildTask) Report += FString::ChrN((Depth + 1) * 2, TEXT(' ')) + Child.ChildTask->GetClass()->GetName() + TEXT(" ") + Child.ChildTask->GetNodeName() + TEXT("\n");
			if (Child.ChildTask) for (UBTService* Service : Child.ChildTask->Services)
				if (Service) Report += FString::ChrN((Depth + 2) * 2, TEXT(' ')) + TEXT("Service ") + Service->GetClass()->GetName() + TEXT(" ") + Service->GetNodeName() + TEXT("\n");
		}
	};
	Walk(Tree->RootNode, 0);
	if (UEdGraph* Graph = Tree->BTGraph)
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			UObject* Instance = nullptr;
			if (FObjectPropertyBase* Property = FindFProperty<FObjectPropertyBase>(Node->GetClass(), TEXT("NodeInstance"))) Instance = Property->GetObjectPropertyValue_InContainer(Node);
			Report += FString::Printf(TEXT("Graph node %s class=%s instance=%s\n"), *Node->GetName(), *Node->GetClass()->GetName(), *GetNameSafe(Instance));
			for (UEdGraphPin* Pin : Node->Pins)
				for (UEdGraphPin* Link : Pin->LinkedTo)
					Report += FString::Printf(TEXT("  %s %s -> %s.%s\n"), Pin->Direction == EGPD_Input ? TEXT("IN") : TEXT("OUT"), *Pin->PinName.ToString(), *Link->GetOwningNode()->GetName(), *Link->PinName.ToString());
		}
	return Report;
}

bool UEnemyBlueprintEditorLibrary::WireMeleeHeadQuery(UBlueprint* Blueprint)
{
	if (!Blueprint) return false;
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	bool bFound = false;
	for (UEdGraph* Graph : Graphs)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node);
			if (!Call) continue;
			if (Call->FunctionReference.GetMemberName() == TEXT("GetEnemyMeleeTargets")) { bFound = true; continue; }
			if (Call->FunctionReference.GetMemberName() != TEXT("GetTargetsWithinRadius")) continue;
			Blueprint->Modify();
			Call->Modify();
			Call->SetFromFunction(URAbilitySystemLibrary::StaticClass()->FindFunctionByName(TEXT("GetEnemyMeleeTargets")));
			Call->ReconstructNode();
			bFound = true;
		}
	}
	if (!bFound) return false;
	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	return Blueprint->Status != BS_Error;
}

bool UEnemyBlueprintEditorLibrary::AddMaceHeadWindow(UAnimMontage* Montage, FGameplayTag EventTag, float StartTime, float EndTime)
{
	if (!Montage || !EventTag.IsValid() || StartTime < 0.f || EndTime <= StartTime || EndTime > Montage->GetPlayLength()) return false;
	for (FAnimNotifyEvent& Existing : Montage->Notifies)
	{
		if (URAnimNotifyState_EnemyMeleeWindow* State = Cast<URAnimNotifyState_EnemyMeleeWindow>(Existing.NotifyStateClass))
		{
			Montage->Modify();
			State->DamageEventTag = EventTag;
			Existing.Link(Montage, StartTime);
			Existing.SetDuration(EndTime - StartTime);
			Existing.EndLink.Link(Montage, EndTime);
			Montage->RefreshCacheData();
			return true;
		}
	}
	Montage->Modify();
	URAnimNotifyState_EnemyMeleeWindow* State = NewObject<URAnimNotifyState_EnemyMeleeWindow>(Montage, NAME_None, RF_Transactional);
	State->DamageEventTag = EventTag;
	FAnimNotifyEvent Event;
	Event.NotifyName = TEXT("Enemy Melee Head Window");
	Event.NotifyStateClass = State;
	Event.TrackIndex = 0;
	Event.Link(Montage, StartTime);
	Event.SetDuration(EndTime - StartTime);
	Event.EndLink.Link(Montage, EndTime);
	Montage->Notifies.Add(Event);
	Montage->SortNotifies();
	Montage->RefreshCacheData();
	return true;
}

FString UEnemyBlueprintEditorLibrary::DescribeMeleeSetup(UBlueprint* Blueprint)
{
	AREnemy* Defaults = Blueprint && Blueprint->GeneratedClass ? Cast<AREnemy>(Blueprint->GeneratedClass->GetDefaultObject()) : nullptr;
	if (!Defaults) return TEXT("Enemy missing");
	FString Report;
	TSet<UAnimSequenceBase*> Visited;
	TFunction<void(UAnimSequenceBase*)> DescribeAnimation = [&](UAnimSequenceBase* Animation)
	{
		if (!Animation || Visited.Contains(Animation)) return;
		Visited.Add(Animation);
		Report += FString::Printf(TEXT("Animation %s length=%.3f\n"), *Animation->GetPathName(), Animation->GetPlayLength());
		for (const FAnimNotifyEvent& Event : Animation->Notifies)
		{
			UObject* Notify = Event.Notify ? static_cast<UObject*>(Event.Notify.Get()) : static_cast<UObject*>(Event.NotifyStateClass.Get());
			Report += FString::Printf(TEXT("Notify %s time=%.3f duration=%.3f class=%s\n"), *Event.NotifyName.ToString(), Event.GetTime(), Event.GetDuration(), *GetNameSafe(Notify ? Notify->GetClass() : nullptr));
			if (!Notify) continue;
			for (TFieldIterator<FStructProperty> It(Notify->GetClass()); It; ++It)
				if (It->Struct == FGameplayTag::StaticStruct())
					Report += FString::Printf(TEXT("  tag %s=%s\n"), *It->GetName(), *It->ContainerPtrToValuePtr<FGameplayTag>(Notify)->ToString());
		}
		if (UAnimMontage* Montage = Cast<UAnimMontage>(Animation))
			for (const FSlotAnimationTrack& Track : Montage->SlotAnimTracks)
				for (const FAnimSegment& Segment : Track.AnimTrack.AnimSegments) DescribeAnimation(Segment.GetAnimReference());
	};
	for (const FTaggedMontage& Entry : Defaults->GetAttackMontages_Implementation())
	{
		Report += FString::Printf(TEXT("Attack montage=%s event=%s socketTag=%s damageSocket=%s\n"), *GetNameSafe(Entry.Montage), *Entry.MontageTag.ToString(), *Entry.SocketTag.ToString(), *Defaults->GetMeleeDamageSocket().ToString());
		DescribeAnimation(Entry.Montage);
	}
	return Report;
}

int32 UEnemyBlueprintEditorLibrary::KeepMeleeMontagePlaying(UBlueprint* Blueprint)
{
	if (!Blueprint) return -1;
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	int32 RemovedLinks = 0;
	bool bHasCompletionExit = false;
	for (UEdGraph* Graph : Graphs)
	{
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			UK2Node_CallFunction* Call = Cast<UK2Node_CallFunction>(Node);
			if (!Call || Call->FunctionReference.GetMemberName() != TEXT("K2_EndAbility")) continue;
			UEdGraphPin* Exec = Call->FindPin(UEdGraphSchema_K2::PN_Execute);
			if (!Exec) continue;
			const TArray<UEdGraphPin*> Links = Exec->LinkedTo;
			for (UEdGraphPin* Link : Links)
			{
				const bool bMontageCallback = Link->GetOwningNode()->GetNodeTitle(ENodeTitleType::FullTitle).ToString().Contains(TEXT("PlayMontageAndWait"))
					&& (Link->PinName == TEXT("OnCompleted") || Link->PinName == TEXT("OnInterrupted") || Link->PinName == TEXT("OnCancelled"));
				if (bMontageCallback)
				{
					bHasCompletionExit |= Link->PinName == TEXT("OnCompleted");
					continue;
				}
				Blueprint->Modify();
				Graph->Modify();
				Call->Modify();
				Exec->BreakLinkTo(Link);
				Call->NodeComment = TEXT("Attack lifetime is owned by PlayMontageAndWait completion/interruption/cancellation.");
				++RemovedLinks;
			}
		}
	}
	if (!bHasCompletionExit) return -1; // Never save a graph without an actual completion exit.
	if (RemovedLinks)
	{
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		if (Blueprint->Status == BS_Error) return -1;
	}
	return RemovedLinks;
}

bool UEnemyBlueprintEditorLibrary::GuardFacingTarget(UBlueprint* Blueprint)
{
	if (!Blueprint) return false;
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	for (UEdGraph* Graph : Graphs)
	{
		if (Graph->GetFName() != TEXT("GetFaceTargetLogic")) continue;
		UK2Node_FunctionEntry* Entry = nullptr;
		UK2Node_FunctionResult* Result = nullptr;
		UEdGraphPin* Target = nullptr;
		UEdGraphPin* OwnLocation = nullptr;
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (Node->NodeComment == TEXT("EnemyFacingTargetGuard")) return true;
			if (auto* Found = Cast<UK2Node_FunctionEntry>(Node)) Entry = Found;
			if (auto* Found = Cast<UK2Node_FunctionResult>(Node)) Result = Found;
			if (auto* Call = Cast<UK2Node_CallFunction>(Node))
			{
				if (Call->FunctionReference.GetMemberName() == TEXT("GetCombatTarget")) Target = Call->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
				if (Call->FunctionReference.GetMemberName() == TEXT("K2_GetActorLocation"))
				{
					UEdGraphPin* Self = Call->FindPin(UEdGraphSchema_K2::PN_Self);
					if (Self && Self->LinkedTo.IsEmpty()) OwnLocation = Call->FindPin(UEdGraphSchema_K2::PN_ReturnValue);
				}
			}
		}
		UEdGraphPin* Start = Entry ? Entry->FindPin(UEdGraphSchema_K2::PN_Then) : nullptr;
		if (!Start || Start->LinkedTo.Num() != 1 || !Result || !Target || !OwnLocation) return false;
		UEdGraphPin* OriginalExec = Start->LinkedTo[0];
		Blueprint->Modify();
		Graph->Modify();
		FGraphNodeCreator<UK2Node_CallFunction> ValidCreator(*Graph);
		UK2Node_CallFunction* Valid = ValidCreator.CreateNode();
		Valid->SetFromFunction(UKismetSystemLibrary::StaticClass()->FindFunctionByName(TEXT("IsValid")));
		Valid->NodePosX = Entry->NodePosX + 200;
		Valid->NodePosY = Entry->NodePosY + 200;
		ValidCreator.Finalize();
		FGraphNodeCreator<UK2Node_IfThenElse> BranchCreator(*Graph);
		UK2Node_IfThenElse* Branch = BranchCreator.CreateNode();
		Branch->NodeComment = TEXT("EnemyFacingTargetGuard");
		Branch->NodePosX = Entry->NodePosX + 400;
		Branch->NodePosY = Entry->NodePosY;
		BranchCreator.Finalize();
		// Preserve the original signature but do not evaluate its target-location
		// expression on the invalid path. Return our own position without rotating.
		UK2Node_FunctionResult* InvalidResult = DuplicateObject<UK2Node_FunctionResult>(Result, Graph);
		InvalidResult->CreateNewGuid();
		for (UEdGraphPin* Pin : InvalidResult->Pins)
		{
			Pin->LinkedTo.Reset();
			Pin->PinId = FGuid::NewGuid();
		}
		InvalidResult->NodePosX = Branch->NodePosX + 300;
		InvalidResult->NodePosY = Branch->NodePosY + 300;
		Graph->AddNode(InvalidResult, false, false);
		const UEdGraphSchema* Schema = Graph->GetSchema();
		Start->BreakAllPinLinks();
		const bool bConnected = Schema->TryCreateConnection(Target, Valid->FindPin(TEXT("Object")))
			&& Schema->TryCreateConnection(Valid->FindPin(UEdGraphSchema_K2::PN_ReturnValue), Branch->GetConditionPin())
			&& Schema->TryCreateConnection(Start, Branch->GetExecPin())
			&& Schema->TryCreateConnection(Branch->GetThenPin(), OriginalExec)
			&& Schema->TryCreateConnection(Branch->GetElsePin(), InvalidResult->FindPin(UEdGraphSchema_K2::PN_Execute))
			&& Schema->TryCreateConnection(OwnLocation, InvalidResult->FindPin(TEXT("OutputPin")));
		if (!bConnected) return false; // Caller must not save a failed repair.
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		return Blueprint->Status != BS_Error;
	}
	return false;
}

FString UEnemyBlueprintEditorLibrary::DescribeFunctionGraph(UBlueprint* Blueprint, FName GraphName)
{
	if (!Blueprint) return TEXT("Blueprint missing");
	TArray<UEdGraph*> Graphs;
	Blueprint->GetAllGraphs(Graphs);
	FString Description;
	for (UEdGraph* Graph : Graphs)
	{
		if (Graph->GetFName() != GraphName) continue;
		Description += FString::Printf(TEXT("Graph %s\n"), *Graph->GetName());
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			Description += FString::Printf(TEXT("Node %s [%s] title=%s\n"), *Node->GetName(), *Node->GetClass()->GetName(), *Node->GetNodeTitle(ENodeTitleType::FullTitle).ToString());
			for (UEdGraphPin* Pin : Node->Pins)
			{
				Description += FString::Printf(TEXT("  %s %s type=%s default=%s\n"), Pin->Direction == EGPD_Input ? TEXT("IN") : TEXT("OUT"), *Pin->PinName.ToString(), *Pin->PinType.PinCategory.ToString(), *Pin->DefaultValue);
				for (UEdGraphPin* Link : Pin->LinkedTo)
					Description += FString::Printf(TEXT("    -> %s.%s\n"), *Link->GetOwningNode()->GetName(), *Link->PinName.ToString());
			}
		}
	}
	return Description.IsEmpty() ? TEXT("Graph missing") : Description;
}
