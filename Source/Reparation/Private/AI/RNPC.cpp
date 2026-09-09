// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/RNPC.h"

#include "RGameplayTags.h"
#include "AI/NPCAIController.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "Framework/RAbilitySystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/RAbilitySystemComponent.h"
#include "GAS/RAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "Reparation/Reparation.h"
#include "UI/GAS/RUserWidget.h"

ARNPC::ARNPC()
{
	AbilitySystemComponent = CreateDefaultSubobject<URAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
	
	AttributeSet = CreateDefaultSubobject<URAttributeSet>("AttributeSet");

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
	
	WeaponRH = CreateDefaultSubobject<USkeletalMeshComponent>("CombatRH");
	WeaponRH->SetupAttachment(GetMesh(), FName("weaponRH"));
	WeaponRH->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	WeaponLH = CreateDefaultSubobject<USkeletalMeshComponent>("CombatLH");
	WeaponLH->SetupAttachment(GetMesh(), FName("weaponLH"));
	WeaponLH->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());
}

void ARNPC::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	AIC_NPC = Cast<ANPCAIController>(NewController);
	AIC_NPC->GetBlackboardComponent()->InitializeBlackboard(*NPCBehaviorTree->BlackboardAsset);
	AIC_NPC->RunBehaviorTree(NPCBehaviorTree);
	AIC_NPC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	//AIC->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), false);
	//AIC->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);
}

int32 ARNPC::GetPlayerLevel_Implementation()
{
	return Level;
}

FVector ARNPC::GetCombatSocketLocation_Implementation(const FGameplayTag& CombatSocketTag)
{
	const FRGameplayTags& GameplayTags = FRGameplayTags::Get();
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_weapon) && IsValid(WeaponRH) || IsValid(WeaponLH))
	{
		if (WeaponRH)
		{
			return WeaponRH->GetSocketLocation(WeaponDamageSocket);
		}
		else
		{
			return WeaponLH->GetSocketLocation(WeaponDamageSocket);
		}
	}
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_handL))
	{
		return GetMesh()->GetSocketLocation(HandRSocket);
	}
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_handR))
	{
		return GetMesh()->GetSocketLocation(HandLSocket);
	}
	if (CombatSocketTag.MatchesTagExact(GameplayTags.combatSocket_jaw))
	{
		return GetMesh()->GetSocketLocation(Jaw);
	}
	return FVector();
}

void ARNPC::Die(const FVector& DeathImpulse)
{
	WeaponRH->SetSimulatePhysics(true);
	WeaponRH->SetEnableGravity(true);
	WeaponRH->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	WeaponRH->AddImpulse(DeathImpulse);

	WeaponLH->SetSimulatePhysics(true);
	WeaponLH->SetEnableGravity(true);
	WeaponLH->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	WeaponLH->AddImpulse(DeathImpulse);
	
	UGameplayStatics::PlaySoundAtLocation(this, DeathCry, GetActorLocation(), GetActorRotation());
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::Type::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
	bDead = true;
	OnDeath.Broadcast(this);
	if (AIC_NPC) AIC_NPC->GetBlackboardComponent()->SetValueAsBool(FName("IsDead"), true);
	
}

void ARNPC::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* ARNPC::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void ARNPC::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (AIC_NPC && AIC_NPC->GetBlackboardComponent())
	{
		AIC_NPC->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void ARNPC::BeginPlay()
{
	Super::BeginPlay();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	InitAbilityActorInfo();
	if (HasAuthority())
	{
		URAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}
	
	if (URUserWidget* EnemyHealthUI = Cast<URUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		EnemyHealthUI->SetWidgetController(this);
	}

	if (const URAttributeSet* AS = Cast<URAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChange.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AS->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChange.Broadcast(Data.NewValue);
			}
		);
		//Call Hit React/Stunned function
		AbilitySystemComponent->RegisterGameplayTagEvent(FRGameplayTags::Get().ability_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ARNPC::HitReactTagChanged);
		//Broadcast Initial Values
		OnHealthChange.Broadcast(AS->GetHealth());
		OnMaxHealthChange.Broadcast(AS->GetMaxHealth());
	}
}

void ARNPC::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<URAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	InitializeAttributes();
}
