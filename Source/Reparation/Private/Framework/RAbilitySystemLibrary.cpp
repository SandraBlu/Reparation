// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/RAbilitySystemLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "RAbilityTypes.h"
#include "RGameplayTags.h"
#include "Framework/RGameMode.h"
#include "Framework/RPlayerState.h"
#include "Interfaces/RCombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "UI/GAS/RHUD.h"
#include "UI/GAS/Controllers/RWidgetController.h"

bool URAbilitySystemLibrary::MakeWidgetControllerParams(const UObject* WorldContextObject,
                                                        FWidgetControllerParams& OutWCParams, ARHUD*& OutRHUD)
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
    	{
		OutRHUD = Cast<ARHUD>(PC->GetHUD());
    		if (OutRHUD)
    		{
    			ARPlayerState* PS = PC->GetPlayerState<ARPlayerState>();
    			UAbilitySystemComponent* ASC = PS->GetAbilitySystemComponent();
    			UAttributeSet* AS = PS->GetAttributeSet();
    			OutWCParams.AttributeSet = AS;
    			OutWCParams.AbilitySystemComponent = ASC;
    			OutWCParams.PlayerState = PS;
    			OutWCParams.PlayerController= PC;
    			FWidgetControllerParams(PC, PS, ASC, AS);
    			return true;
    		}
    	}
    return false;
}

UROverlayWidgetController* URAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{

	FWidgetControllerParams WCParams;
	ARHUD* RHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, RHUD))
	{
		return RHUD->GetOverlayWidgetController(WCParams);
	}
	return nullptr;
}

URAttributeMenuController* URAbilitySystemLibrary::GetAttributeMenuController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARHUD* RHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, RHUD))
	{
		return RHUD->GetAttributeMenuWidgetController(WCParams);
	}
	return nullptr;
}

URAbilityMenuController* URAbilitySystemLibrary::GetAbilityMenuController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WCParams;
	ARHUD* RHUD = nullptr;
	if (MakeWidgetControllerParams(WorldContextObject, WCParams, RHUD))
	{
			return RHUD->GetAbilityMenuWidgetController(WCParams);
	}
	return nullptr;
}

void URAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject,
                                                         ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* ASC)
{
	AActor* AvatarActor = ASC->GetAvatarActor();
	
	URCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	FCharClassInfo ClassDefaultInfo = CharacterClassInfo->GetClassInfo(CharacterClass);

	//Add Source Objects(ASC) and Effect Context and Outgoing Spec for AI Attributes Data Asset: CharacterClassInfo
	FGameplayEffectContextHandle PrimaryAttributeContextHandle = ASC->MakeEffectContext();
	PrimaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle PrimaryAttSpecHandle = ASC->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*PrimaryAttSpecHandle.Data.Get());

	FGameplayEffectContextHandle SecondaryAttributeContextHandle = ASC->MakeEffectContext();
	SecondaryAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle SecondaryAttSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*SecondaryAttSpecHandle.Data.Get());

	FGameplayEffectContextHandle ResistanceAttributeContextHandle = ASC->MakeEffectContext();
	ResistanceAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle ResistanceAttSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->ResistanceAttributes, Level, ResistanceAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*ResistanceAttSpecHandle.Data.Get());

	FGameplayEffectContextHandle VitalAttributeContextHandle = ASC->MakeEffectContext();
	VitalAttributeContextHandle.AddSourceObject(AvatarActor);
	const FGameplayEffectSpecHandle VitalAttSpecHandle = ASC->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributeContextHandle);
	ASC->ApplyGameplayEffectSpecToSelf(*VitalAttSpecHandle.Data.Get());
}

void URAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject, UAbilitySystemComponent* ASC, ECharacterClass CharacterClass)
{
	URCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return;
	for (TSubclassOf<UGameplayAbility> AbilityClass : CharacterClassInfo->SharedAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		ASC->GiveAbility(AbilitySpec);
	}
	const FCharClassInfo& DefaultInfo = CharacterClassInfo->GetClassInfo(CharacterClass);
	for (auto AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (ASC->GetAvatarActor()->Implements<URCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, IRCombatInterface::Execute_GetPlayerLevel(ASC->GetAvatarActor()));
			ASC->GiveAbility(AbilitySpec);
		}
	}
}

URCharacterClassInfo* URAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const ARGameMode* RGameMode = Cast<ARGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (RGameMode == nullptr) return nullptr;
	return RGameMode->CharacterClassInfo;
}

UAbilityInfo* URAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const ARGameMode* RGameMode = Cast<ARGameMode>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (RGameMode == nullptr) return nullptr;
	return RGameMode->AbilityInfo;
}

int32 URAbilitySystemLibrary::GetXPRewardForEnemySlay(const UObject* WorldContextObject, ECharacterClass CharacterClass,
                                                      int32 CharacterLevel)
{
	URCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (CharacterClassInfo == nullptr) return 0;
	const FCharClassInfo& Info = CharacterClassInfo->GetClassInfo(CharacterClass);
	const float XPReward = Info.XPReward.GetValueAtLevel(CharacterLevel);

	return static_cast<int32>(XPReward);
}

FGameplayEffectContextHandle URAbilitySystemLibrary::ApplyDamageEffect(const FDamageEffectParams& DamageEffectParams)
{
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
	const AActor* SourceAvatarActor = DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor();

	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);
	SetKnockbackForce(EffectContextHandle, DamageEffectParams.KnockbackForce);
	SetIsRadialDamage(EffectContextHandle, DamageEffectParams.bIsRadialDamage);
	SetRadialDamageInnerRadius(EffectContextHandle, DamageEffectParams.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(EffectContextHandle, DamageEffectParams.RadialDamageOuterRadius);
	SetRadialDamageOrigin(EffectContextHandle, DamageEffectParams.RadialDamageOrigin);
	
	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageType, DamageEffectParams.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);

	DamageEffectParams.TargetAbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);
	return EffectContextHandle;
}

TArray<FRotator> URAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis,float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumRotators > 1)
	{
		const float DeltaSpread = Spread / (NumRotators - 1);
    	for (int32 i = 0; i < NumRotators; i++)
    	{
    		const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
    		Rotators.Add(Direction.Rotation());
    	}	
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}
	return Rotators;
}

TArray<FVector> URAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);
	if (NumVectors > 1)
	{
		const float DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; i++)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, FVector::UpVector);
			Vectors.Add(Direction);
		}	
	}
	else
	{
		Vectors.Add(Forward);
	}
	return Vectors;
}

bool URAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->IsBlockedHit();
	}
	return false;
}

bool URAbilitySystemLibrary::IsDodgedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->IsDodgedHit();
	}
	return false;
}

bool URAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->IsCriticalHit();
	}
	return false;
}

bool URAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->IsSuccessfulDebuff();
	}
	return false;
}

float URAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float URAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float URAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

FGameplayTag URAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (REffectContext->GetDamageType().IsValid())
		{
			return *REffectContext->GetDamageType();
		}
	}
	return FGameplayTag();
}

FVector URAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

FVector URAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->GetKnockbackForce();
	}
	return FVector::ZeroVector;
}

bool URAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->IsRadialDamage();
	}
	return false;
}

float URAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->GetRadialDamageInnerRadius();
	}
	return 0.f;
}

float URAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->GetRadialDamageOuterRadius();
	}
	return 0.f;
}

FVector URAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FRGameplayEffectContext* REffectContext = static_cast<const FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return REffectContext->GetRadialDamageOrigin();
	}
	return FVector::ZeroVector;
}

void URAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsBlockedHit)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void URAbilitySystemLibrary::SetIsDodgedHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsDodgedHit)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetIsDodgedHit(bInIsDodgedHit);
	}
}

void URAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsCriticalHit)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void URAbilitySystemLibrary::SetIsSucessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle, bool bInSuccessfulDebuff)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetIsSuccessfulDebuff(bInSuccessfulDebuff);
	}
}

void URAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, float InDebuffDamage)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetDebuffDamage(InDebuffDamage);
	}
}

void URAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle, float InDebuffDuration)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetDebuffDuration(InDebuffDuration);
	}
}

void URAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle, float InDebuffFrequency)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetDebuffFrequency(InDebuffFrequency);
	}
}

void URAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		const TSharedPtr<FGameplayTag> DamageType = MakeShared<FGameplayTag>(InDamageType);
		REffectContext->SetDamageType(DamageType);
	}
}

void URAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,	const FVector& InImpulse)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetDeathImpulse(InImpulse);
	}
}

void URAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InForce)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetKnockbackForce(InForce);
	}
}

void URAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void URAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle, float InInnerRadius)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetRadialDamageInnerRadius(InInnerRadius);
	}
}

void URAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle, float InOuterRadius)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetRadialDamageOuterRadius(InOuterRadius);
	}
}

void URAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle, const FVector& InOrigin)
{
	if (FRGameplayEffectContext* REffectContext = static_cast<FRGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		REffectContext->SetRadialDamageOrigin(InOrigin);
	}
}

void URAbilitySystemLibrary::GetTargetsWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors,
                                                    const TArray<AActor*>& ActorsToIgnore, float Radius, const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);

	TArray<FOverlapResult> Overlaps;
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<URCombatInterface>() && !IRCombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(IRCombatInterface::Execute_GetAvatar(Overlap.GetActor()));
			}
		}
	}
}

bool URAbilitySystemLibrary::IsNotFriend(AActor* FirstActor, AActor* SecondActor)
{
	const bool bPlayerTeam = FirstActor->ActorHasTag(FName("player")) && SecondActor->ActorHasTag(FName("player"));
	const bool bEnemyTeam = FirstActor->ActorHasTag(FName("enemy")) && SecondActor->ActorHasTag(FName("enemy"));
	const bool bFriend = bPlayerTeam || bEnemyTeam;
	return !bFriend;
}

