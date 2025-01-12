// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/RItem.h"
#include "Components/RInventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "UObject/UnrealType.h"
#include "Engine/BlueprintGeneratedClass.h"

#define LOCTEXT_NAMESPACE "Item"

#define ItemStat_DisplayName "DisplayName"
#define ItemStat_Weight "Weight"
#define ItemStat_Quantity "Quantity"
#define ItemStat_RechargeDuration "RechargeDuration"
#define ItemStat_StackWeight "StackWeight"
#define ItemStat_MaxStackSize "MaxStackSize"
#define ItemStat_BaseValue "BaseValue"

URItem::URItem()
{
	DisplayName = LOCTEXT("ItemName", "Item");
	UseActionText = LOCTEXT("UseActionText", "Use");
	Weight = 0.f;
	bStackable = true;
	Quantity = 1;
	MaxStackSize = 2;
	RepKey = 0;
	LastUseTime = -FLT_MAX;
	BaseValue = 10;

	FString NameString = GetName();

	//Add the Default Speaker to the Quest 
	int32 UnderscoreIndex = -1;

	if (NameString.FindChar(TCHAR('_'), UnderscoreIndex))
	{
		//remove item name prefix
		DisplayName = FText::FromString(FName::NameToDisplayString(NameString.RightChop(UnderscoreIndex + 1), false));
	}
	else
	{
		DisplayName = FText::FromString(NameString);
	}

	Stats.Add(FItemStat(LOCTEXT("WeightStatDisplayText", "Weight"), ItemStat_Weight));
	Stats.Add(FItemStat(LOCTEXT("QuantityStatDisplayText", "Quantity"), ItemStat_Quantity));
}

void URItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	//Inventory needs items to support user added blueprint replicated variables 
	UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(GetClass());
	if (BPClass != NULL)
	{
		BPClass->GetLifetimeBlueprintReplicationList(OutLifetimeProps);
	}

	DOREPLIFETIME(URItem, Quantity);
	DOREPLIFETIME_CONDITION_NOTIFY(URItem, bActive, COND_None, REPNOTIFY_OnChanged);
	DOREPLIFETIME_CONDITION(URItem, LastUseTime, COND_OwnerOnly);
}

bool URItem::IsSupportedForNetworking() const
{
	return true;
}

UWorld* URItem::GetWorld() const
{
	return World;
}

#if WITH_EDITOR
void URItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName ChangedPropertyName = PropertyChangedEvent.Property ? PropertyChangedEvent.Property->GetFName() : NAME_None;

	//UPROPERTY clamping doesn't support using a variable to clamp so we do in here instead
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(URItem, Quantity))
	{
		Quantity = FMath::Clamp(Quantity, 1, GetMaxStackSize());
	}
	else if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(URItem, bStackable))
	{
		if (!bStackable)
		{
			Quantity = 1;
		}
	}
}
#endif

bool URItem::HasAuthority() const
{
	return (OwningInventory && OwningInventory->GetOwnerRole() >= ROLE_Authority);
}

void URItem::OnRep_bActive(const bool bOldActive)
{
	if (bActive)
	{
		Activated();
	}
	else
	{
		Deactivated();
	}
}

void URItem::SetActive(const bool bNewActive, const bool bForce)
{
	if (bCanActivate)
	{
		if (bNewActive != bActive || bForce)
		{
			bActive = bNewActive;
			OnRep_bActive(!bActive);
			MarkDirtyForReplication();
		}
	}
}

void URItem::OnRep_Quantity(const int32 OldQuantity)
{
	OnItemModified.Broadcast();
}

void URItem::SetQuantity(const int32 NewQuantity)
{
	if (NewQuantity != Quantity)
	{
		const int32 OldQuantity = Quantity;
		Quantity = FMath::Clamp(NewQuantity, 0, GetMaxStackSize());
		OnRep_Quantity(OldQuantity);
		MarkDirtyForReplication();
	}
}

void URItem::SetLastUseTime(const float NewLastUseTime)
{
	LastUseTime = NewLastUseTime;
	MarkDirtyForReplication();
}

void URItem::Use()
{
}

void URItem::AddedToInventory(URInventoryComponent* Inventory)
{

}

void URItem::RemovedFromInventory(URInventoryComponent* Inventory)
{
	if (bActive)
	{
		Deactivated();
	}
}

void URItem::MarkDirtyForReplication()
{
	//Mark this object for replication
	++RepKey;

	//Mark the array for replication
	if (OwningInventory)
	{
		++OwningInventory->ReplicatedItemsKey;
	}
}

bool URItem::ShouldShowInInventory_Implementation() const
{
	//By default, don't show vendors equipped items in their store  
	if (OwningInventory && OwningInventory->bIsVendor)
	{
		if (bActive)
		{
			return false; 
		}
	}

	return true;
}

bool URItem::CanBeRemoved_Implementation() const
{
	return true;
}

void URItem::Activated_Implementation()
{
}

void URItem::Deactivated_Implementation()
{
}

bool URItem::CanUse_Implementation() const
{
	return true;
}

bool URItem::ShouldUseOnAdd_Implementation() const
{
	return false;
}

FText URItem::GetRawDescription_Implementation()
{
	return Description;
}

FText URItem::GetParsedDescription()
{
	//Replace variables in dialogue line
	FString LineString = Description.ToString();

	int32 OpenBraceIdx = -1;
	int32 CloseBraceIdx = -1;
	bool bFoundOpenBrace = LineString.FindChar('{', OpenBraceIdx);
	bool bFoundCloseBrace = LineString.FindChar('}', CloseBraceIdx);
	uint32 Iters = 0; // More than 50 wildcard replaces and something has probably gone wrong, so safeguard against that

	while (bFoundOpenBrace && bFoundCloseBrace && OpenBraceIdx < CloseBraceIdx && Iters < 50)
	{
		const FString VariableName = LineString.Mid(OpenBraceIdx + 1, CloseBraceIdx - OpenBraceIdx - 1);
		const FString VariableVal = GetStringVariable(VariableName);

		if (!VariableVal.IsEmpty())
		{
			LineString.RemoveAt(OpenBraceIdx, CloseBraceIdx - OpenBraceIdx + 1);
			LineString.InsertAt(OpenBraceIdx, VariableVal);
		}

		bFoundOpenBrace = LineString.FindChar('{', OpenBraceIdx);
		bFoundCloseBrace = LineString.FindChar('}', CloseBraceIdx);

		Iters++;
	}

	return FText::FromString(LineString);
}

FString URItem::GetStringVariable_Implementation(const FString& VariableName)
{
	//Overriable in BP in case you want to add more 
	if (VariableName == ItemStat_DisplayName)
	{
		return DisplayName.ToString();
	}
	else if (VariableName == ItemStat_Weight)
	{
		return FString::SanitizeFloat(Weight);
	}
	else if (VariableName == ItemStat_RechargeDuration)
	{
		return FString::SanitizeFloat(UseRechargeDuration);
	}
	else if (VariableName == ItemStat_StackWeight)
	{
		return FString::SanitizeFloat(GetStackWeight());
	}
	else if (VariableName == ItemStat_Quantity)
	{
		return FString::FromInt(Quantity);
	}
	else if (VariableName == ItemStat_MaxStackSize)
	{
		return FString::FromInt(MaxStackSize);
	}
	else if (VariableName == ItemStat_BaseValue)
	{
		return FString::FromInt(BaseValue);
	}

	return FString();
}

APlayerController* URItem::GetOwningController() const
{
	if (OwningInventory)
	{
		return OwningInventory->GetOwningController();
	}

	return nullptr;
}

APawn* URItem::GetOwningPawn() const
{
	if (OwningInventory)
	{
		return OwningInventory->GetOwningPawn();
	}
	return nullptr;
}

#undef LOCTEXT_NAMESPACE