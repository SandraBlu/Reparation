// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RPlayerState.h"

#include "Components/RInventoryComponent.h"
#include "Framework/RGameMode.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "RTypes/REnumTypes.h"
#include "GAS/Data/LevelUpInfo.h"
#include "Framework/RPIETestSettings.h"
#include "Engine/World.h"

ARPlayerState::ARPlayerState()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	AttributeSet = CreateDefaultSubobject<URAttributeSet>("AttributeSet");

	PlayerInventory = CreateDefaultSubobject<URInventoryComponent>("InventoryComp");
	PlayerInventory->SetCapacity(25);
	PlayerInventory->SetWeightCapacity(60.f);
	
	SetNetUpdateFrequency(100.f);
}

void ARPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARPlayerState, Level);
	DOREPLIFETIME(ARPlayerState, XP);
	DOREPLIFETIME(ARPlayerState, AttributePoints);
	DOREPLIFETIME(ARPlayerState, AbilityPoints);
}

UAbilitySystemComponent* ARPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ARPlayerState::AddToXP(int32 InXP)
{
	XP = static_cast<int32>(FMath::Clamp<int64>(static_cast<int64>(XP) + InXP, 0, MAX_int32));
	OnXPChangeDelegate.Broadcast(XP);
}

void ARPlayerState::AddToLevel(int32 InLevel)
{
	Level = static_cast<int32>(FMath::Clamp<int64>(static_cast<int64>(Level) + InLevel, 1, LevelUpInfo ? LevelUpInfo->GetMaxLevel() : MAX_int32));
	if (URAbilitySystemComponent* ASC = Cast<URAbilitySystemComponent>(AbilitySystemComponent)) ASC->RefreshPassiveSlotLimits();
	OnLevelChangeDelegate.Broadcast(Level);
}

void ARPlayerState::AddToAttributePts(int32 InAttributePts)
{
	AttributePoints = static_cast<int32>(FMath::Clamp<int64>(static_cast<int64>(AttributePoints) + InAttributePts, 0, MAX_int32));
	OnAttributePtsChangeDelegate.Broadcast(AttributePoints);
}

void ARPlayerState::AddToAbilityPts(int32 InAbilityPts)
{
	AbilityPoints = static_cast<int32>(FMath::Clamp<int64>(static_cast<int64>(AbilityPoints) + InAbilityPts, 0, MAX_int32));
	OnAbilityPtsChangeDelegate.Broadcast(AbilityPoints);
}

void ARPlayerState::SetXP(int32 InXP)
{
	XP = FMath::Max(0, InXP);
	OnXPChangeDelegate.Broadcast(XP);
}

void ARPlayerState::SetLevel(int32 InLevel)
{
	Level = FMath::Clamp(InLevel, 1, LevelUpInfo ? LevelUpInfo->GetMaxLevel() : MAX_int32);
	if (URAbilitySystemComponent* ASC = Cast<URAbilitySystemComponent>(AbilitySystemComponent)) ASC->RefreshPassiveSlotLimits();
	OnLevelChangeDelegate.Broadcast(Level);
}

void ARPlayerState::ApplyPIETestOverrides()
{
#if WITH_EDITOR
	const URPIETestSettings* Settings = GetDefault<URPIETestSettings>();
	if (!HasAuthority() || !GetWorld() || GetWorld()->WorldType != EWorldType::PIE
		|| bPIETestOverridesApplied || !Settings->bEnableTestOverrides) return;
	if (!LevelUpInfo || LevelUpInfo->LevelUpInfo.Num() < 2)
	{
		UE_LOG(LogTemp, Warning, TEXT("PIE testing: progression data is missing; test overrides skipped."));
		return;
	}
	bPIETestOverridesApplied = true;
	const int32 TestLevel = FMath::Clamp(Settings->StartingLevel, 1, FMath::Min(40, LevelUpInfo->GetMaxLevel()));
	SetLevel(TestLevel);
	// Match the start of this level without awarding intermediate level-up rewards.
	SetXP(LevelUpInfo->LevelUpInfo[TestLevel - 1].LevelUpRequirement);
	AddToAbilityPts(FMath::Max(0, Settings->StartingSkillPoints) - AbilityPoints);
	AddToAttributePts(FMath::Max(0, Settings->StartingAttributePoints) - AttributePoints);
	if (PlayerInventory) PlayerInventory->SetCurrency(FMath::Max(0, Settings->StartingGold));
	UE_LOG(LogTemp, Display, TEXT("PIE testing active: level %d, XP %d, skill points %d, attribute points %d. Disable in Project Settings > Game > PIE Testing and restart PIE for normal progression."), Level, XP, AbilityPoints, AttributePoints);
#endif
}

void ARPlayerState::OnRep_Level(int32 OldLevel)
{
	OnLevelChangeDelegate.Broadcast(Level);
}

void ARPlayerState::OnRep_XP(int32 OldXP)
{
	OnXPChangeDelegate.Broadcast(XP);
}

void ARPlayerState::OnRep_AttributePoints(int32 OldAttributePoints)
{
	OnAttributePtsChangeDelegate.Broadcast(AttributePoints);
}

void ARPlayerState::OnRep_AbilityPoints(int32 OldAbilityPoints)
{
	OnAbilityPtsChangeDelegate.Broadcast(AbilityPoints);
}
